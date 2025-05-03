#include <android/log.h>
#include "RecordingEngine.h"
#include "../../../../oboe/include/oboe/Oboe.h"
#include <inttypes.h>  // For PRId64

namespace little_endian_io
{
    template <typename Word>
    std::ostream& write_word(std::ostream& outs, Word value, unsigned size = sizeof(Word))
    {
        for (; size; --size, value >>= 8)
            outs.put(static_cast<char>(value & 0xFF));
        return outs;
    }
}

using namespace little_endian_io;

long long currentTimeMillisRecording() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

RecordingEngine::RecordingEngine() {
    isRecording = false;
    isPaused = false;
    firstFrameHit = false;
    framePosition = 0;
    presentationTime = 0;
}

void RecordingEngine::startRecording(const char* filePath, oboe::InputPreset inputPreset, long startRecordingTimestamp) {
    // Store the file path
    path = filePath;

    __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "Starting recording to file: %s", path);

    // Set state flags
    isRecording = true;
    isPaused = false;
    firstFrameHit = false;

    // Create and open the WAV file
    std::ofstream f;
    f.open(path, std::ios::binary);

    if (!f.is_open()) {
        __android_log_print(ANDROID_LOG_ERROR, "OboeAudioRecorder", "Failed to open file for writing: %s", path);
        return;
    }

    int bitsPerSample = 16;
    int numChannels = 1;

    // Write WAV header
    f << "RIFF----WAVEfmt ";
    write_word(f, 16, 4);  // No extension data
    write_word(f, 1, 2);   // PCM - integer samples
    write_word(f, numChannels, 2);  // Mono
    write_word(f, 44100, 4);  // Sample rate
    write_word(f, (44100 * bitsPerSample * numChannels) / 8, 4);  // Byte rate
    write_word(f,  4, 2);  // Block align
    write_word(f, bitsPerSample, 2);  // Bits per sample

    // Write data chunk header
    f << "data----";
    f.flush();

    // Create audio stream builder
    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Input)
            ->setPerformanceMode(oboe::PerformanceMode::None)
            ->setFormat(oboe::AudioFormat::I16)
            ->setChannelCount(oboe::ChannelCount::Mono)
            ->setInputPreset(inputPreset)
            ->setSharingMode(oboe::SharingMode::Exclusive)
            ->setSampleRate(44100)
            ->setSampleRateConversionQuality(oboe::SampleRateConversionQuality::Best);

    // Open the stream
    oboe::Result r = builder.openStream(&stream);
    if (r != oboe::Result::OK) {
        __android_log_print(ANDROID_LOG_ERROR, "OboeAudioRecorder", "Failed to open stream: %s", oboe::convertToText(r));
        f.close();
        return;
    }

    // Start the stream
    r = stream->requestStart();
    if (r != oboe::Result::OK) {
        __android_log_print(ANDROID_LOG_ERROR, "OboeAudioRecorder", "Failed to start stream: %s", oboe::convertToText(r));
        stream->close();
        f.close();
        return;
    }

    // Check if stream started successfully
    if (stream->getState() != oboe::StreamState::Started) {
        __android_log_print(ANDROID_LOG_ERROR, "OboeAudioRecorder", "Stream did not start properly");
        stream->close();
        f.close();
        return;
    }

    __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "Stream started successfully at %lld ms",
                        currentTimeMillisRecording());

    // Set up for recording
    constexpr int kMillisecondsToRecord = 20;
    auto requestedFrames = (int32_t)(kMillisecondsToRecord * (stream->getSampleRate() / oboe::kMillisPerSecond));
    __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "requestedFrames = %d", requestedFrames);

    // Buffer for audio data
    int16_t* mybuffer = new int16_t[requestedFrames];
    constexpr int64_t kTimeoutValue = 3 * oboe::kNanosPerMillisecond;

    // Main recording loop - run in the current thread for simplicity
    while (isRecording) {
        // Only process audio when not paused
        if (!isPaused) {
            auto result = stream->read(mybuffer, requestedFrames, kTimeoutValue * 1000);

            // Handle timestamp on first frame
            if (!firstFrameHit) {
                oboe::Result timestampResult = stream->getTimestamp(CLOCK_BOOTTIME, &framePosition, &presentationTime);
                if (timestampResult == oboe::Result::OK) {
                    presentationTime = currentTimeMillisRecording();
                    __android_log_print(ANDROID_LOG_INFO, "OboeAudio", "First frame timestamp: %" PRId64 " ms", presentationTime);
                }
                firstFrameHit = true;
            }

            if (result == oboe::Result::OK) {
                int framesRead = result.value();
                // Write the audio data to file
                for (int i = 0; i < framesRead; i++) {
                    write_word(f, (int)(mybuffer[i]), 2);
                }

                // Make sure data is written to disk
                f.flush();
            } else {
                __android_log_print(ANDROID_LOG_ERROR, "OboeAudioRecorder", "Error reading: %s",
                                    oboe::convertToText(result.error()));

                // Handle disconnections
                if (result.error() == oboe::Result::ErrorDisconnected) {
                    __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "Device disconnected");
                    // We would handle reconnection here in a production app
                    handleStreamDisconnection();
                }
            }
        } else {
            // When paused, sleep briefly to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    // Clean up
    delete[] mybuffer;

    // Stop and close the stream
    stream->requestStop();
    stream->close();

    // Update WAV header with final sizes
    long fileSize = f.tellp();
    f.seekp(4, std::ios::beg);
    write_word(f, fileSize - 8, 4);

    f.seekp(40, std::ios::beg);
    write_word(f, fileSize - 44, 4);

    // Close the file
    f.close();

    __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "Recording completed. File size: %ld bytes", fileSize);
}

void RecordingEngine::stopRecording() {
    __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "Stop recording requested");
    isRecording = false;
}

void RecordingEngine::pauseRecording() {
    __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "Pausing recording");
    isPaused = true;
}

void RecordingEngine::resumeRecording() {
    __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "Resuming recording");
    isPaused = false;
}

jlong RecordingEngine::getFramePosition() {
    return framePosition;
}

jlong RecordingEngine::getFrameTimeStamp() {
    return presentationTime;
}

jint RecordingEngine::getAudioSessionId() {
    return stream ? stream->getSessionId() : -1;
}

void RecordingEngine::handleStreamDisconnection() {
    __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "Handling stream disconnection...");

    // Stop the stream but DO NOT close the file
    isRecording = false;

    if (stream) {
        stream->requestStop();
        stream->close();
        stream = nullptr;
    }

    // Wait for device reconnection
    int retryCount = 0;
    constexpr int kMaxRetries = 10;
    while (retryCount < kMaxRetries) {
        __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "Waiting for device reconnection (%d/%d)...",
                            retryCount + 1, kMaxRetries);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Attempt to restart the stream while keeping the file open
        if (attemptStreamRecovery()) {
            __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "Reconnection successful!");
            return;
        }

        retryCount++;
    }

    __android_log_print(ANDROID_LOG_ERROR, "OboeAudioRecorder", "Failed to recover from disconnection.");
}

bool RecordingEngine::attemptStreamRecovery() {
    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Input)
            ->setPerformanceMode(oboe::PerformanceMode::None)
            ->setFormat(oboe::AudioFormat::I16)
            ->setChannelCount(oboe::ChannelCount::Mono)
            ->setInputPreset(oboe::InputPreset::VoiceRecognition)
            ->setSharingMode(oboe::SharingMode::Exclusive)
            ->setSampleRate(44100);

    oboe::Result result = builder.openStream(&stream);
    if (result != oboe::Result::OK) {
        __android_log_print(ANDROID_LOG_ERROR, "OboeAudioRecorder", "Failed to reopen stream: %s", oboe::convertToText(result));
        return false;
    }

    result = stream->requestStart();
    if (result != oboe::Result::OK) {
        __android_log_print(ANDROID_LOG_ERROR, "OboeAudioRecorder", "Failed to start recovered stream: %s", oboe::convertToText(result));
        stream->close();
        return false;
    }

    isRecording = true;
    return true;
}

