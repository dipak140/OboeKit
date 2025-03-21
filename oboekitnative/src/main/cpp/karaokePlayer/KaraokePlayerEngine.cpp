//
// Created by Dipak Sisodiya on 21/03/25.
//

#include "KaraokePlayerEngine.h"
#include <android/log.h>
#include <inttypes.h>
#include <chrono>
#include <jni.h>

static
const char * TAG = "KaraokePlayerEngine";
using namespace oboe;
using namespace parselib;

namespace iolib {
    constexpr int32_t kBufferSizeInBursts = 2; // Use 2 bursts as the buffer size (double buffer)

    KaraokePlayerEngine::KaraokePlayerEngine(): mChannelCount(0), mSampleRate(0), mNumSampleBuffers(0), mOutputReset(false) {}

    DataCallbackResult KaraokePlayerEngine::MyDataCallback::onAudioReady(
            AudioStream * oboeStream, void * audioData, int32_t numFrames) {

        StreamState streamState = oboeStream -> getState();

        // Capture the first frame's timestamp
        if (!this -> mParent -> firstFrameHit) {
            mParent -> presentationTime = std::chrono::duration_cast < std::chrono::milliseconds > (
                    std::chrono::system_clock::now().time_since_epoch()).count();
            mParent -> framePosition = numFrames;
            this -> mParent -> firstFrameHit = true;
        }

        // Log the stream state
        if (streamState != StreamState::Open && streamState != StreamState::Started) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "StreamState: %d", streamState);
        }

        // Handle disconnected state
        if (streamState == StreamState::Disconnected) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "StreamState::Disconnected");
            return DataCallbackResult::Stop;
        }

        // Handle paused state
        if (streamState == StreamState::Paused) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "StreamState::Paused");
            return DataCallbackResult::Continue;
        }

        // Clear the audio buffer
        memset(audioData, 0, static_cast < size_t > (numFrames) *
                             static_cast < size_t > (mParent -> mChannelCount) * sizeof(float));

        // Mix audio from sample sources
        for (int32_t index = 0; index < mParent -> mNumSampleBuffers; index++) {
            if (mParent -> mSampleSources[index] -> isPlaying()) {
                mParent -> mSampleSources[index] -> mixAudio(
                        static_cast < float * > (audioData), mParent -> mChannelCount, numFrames);
            }
        }

        return DataCallbackResult::Continue;
    }

    long long currentTimeMillis() {
        return std::chrono::duration_cast < std::chrono::milliseconds > (
                std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    void KaraokePlayerEngine::MyErrorCallback::onErrorAfterClose(AudioStream * oboeStream, Result error) {
        __android_log_print(ANDROID_LOG_INFO, TAG, "==== onErrorAfterClose() error:%d", error);

        // Store current playback state and positions
        struct PlaybackState {
            int32_t sourceIndex;
            int32_t position;
            float gain;
            float pan;
        };

        std::vector < PlaybackState > activeSourceStates;

        // First store all playing sources' states
        for (int32_t i = 0; i < mParent -> mNumSampleBuffers; i++) {
            if (mParent -> mSampleSources[i] -> isPlaying()) {
                PlaybackState state;
                state.sourceIndex = i;
                state.position = mParent -> mSampleSources[i] -> mCurSampleIndex;
                state.gain = mParent -> mSampleSources[i] -> getGain();
                state.pan = mParent -> mSampleSources[i] -> getPan();
                activeSourceStates.push_back(state);

                __android_log_print(ANDROID_LOG_INFO, TAG,
                                    "Stored state for source %d: pos=%d, gain=%f, pan=%f",
                                    i, state.position, state.gain, state.pan);
            }
        }

        // Clean up existing stream
        if (mParent -> mAudioStream) {
            mParent -> mAudioStream -> stop();
            mParent -> mAudioStream -> close();
            mParent -> mAudioStream.reset();
        }

        usleep(50000); // 50ms pause

        // Attempt stream restoration
        int retryCount = 0;
        const int maxRetries = 3;
        bool streamRestored = false;

        while (retryCount < maxRetries && !streamRestored) {
            __android_log_print(ANDROID_LOG_INFO, TAG, "Attempting stream restore, try %d", retryCount + 1);

            AudioStreamBuilder builder;

            Result result = builder.setChannelCount(mParent -> mChannelCount)
                            -> setDirection(Direction::Output)
                            -> setFormat(AudioFormat::Float)
                            ->  setSharingMode(SharingMode::Shared)
                            -> setPerformanceMode(PerformanceMode::LowLatency)
                            -> setFramesPerCallback(mParent -> mAudioStream?mParent -> mAudioStream -> getFramesPerBurst() : 192)
                            -> setDataCallback(mParent -> mDataCallback)
                            -> setErrorCallback(mParent -> mErrorCallback)
                            -> openStream(mParent -> mAudioStream);

            if (result == Result::OK && mParent -> mAudioStream) {
                mParent -> mAudioStream -> setBufferSizeInFrames(
                        mParent -> mAudioStream -> getFramesPerBurst() * kBufferSizeInBursts);

                mParent -> mSampleRate = mParent -> mAudioStream -> getSampleRate();

                result = mParent -> mAudioStream -> requestStart();
                if (result == Result::OK) {
                    __android_log_print(ANDROID_LOG_INFO, TAG, "Stream restored successfully");
                    streamRestored = true;
                    mParent -> mOutputReset = true;
                    break;
                }
            }

            if (mParent -> mAudioStream) {
                mParent -> mAudioStream -> close();
                mParent -> mAudioStream.reset();
            }

            retryCount++;
            if (retryCount < maxRetries) {
                usleep(100000 * (1 << retryCount));
            }
        }

        if (!streamRestored) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Failed to restore stream after %d attempts", maxRetries);
            return;
        }

        // Restore playback states
        if (streamRestored && !activeSourceStates.empty()) {
            try {
                for (const auto & state: activeSourceStates) {
                    __android_log_print(ANDROID_LOG_INFO, TAG,
                                        "Restoring source %d to position %d",
                                        state.sourceIndex, state.position);
                    mParent -> triggerDown(state.sourceIndex);
                    // First seek to the stored position
                    mParent -> mSampleSources[state.sourceIndex] -> seekToFrame(state.position);
                    // Restore gain and pan
                    mParent -> mSampleSources[state.sourceIndex] -> setGain(state.gain);
                    mParent -> mSampleSources[state.sourceIndex] -> setPan(state.pan);
                }
            } catch (const std::exception & e) {
                __android_log_print(ANDROID_LOG_ERROR, TAG, "Error restoring playback: %s", e.what());
            }
        }
    }

    int64_t KaraokePlayerEngine::getFramePosition() {
        return framePosition;
    }

    int64_t KaraokePlayerEngine::getFrameTimeStamp() {
        return presentationTime;
    }

    void KaraokePlayerEngine::seekTo(int64_t positionMillis, int sampleRate, int channel) {
        if (positionMillis < 0) {
            return;
        }

        int64_t frameOffset = (positionMillis * sampleRate * channel) / 1000;

        // Apply the seek to all active sample sources
        for (int32_t i = 0; i < mNumSampleBuffers; ++i) {
            if (mSampleSources[i] -> isPlaying()) {
                mSampleSources[i] -> seekToFrame(frameOffset);
            }
        }
    }

    int64_t KaraokePlayerEngine::getPosition(int index, int sampleRate, int channelCount) {
        return mSampleSources[index] -> getCurrentPositionInMillis(sampleRate, channelCount);
    }

    int64_t KaraokePlayerEngine::getDuration(int i, int channelCount) {
        return mSampleSources[0] -> getDurationInMillis(i, channelCount);
    }

    int KaraokePlayerEngine::getAudioSessionId() {
        return mAudioStream -> getSessionId();
    }

    bool KaraokePlayerEngine::openStream() {
        __android_log_print(ANDROID_LOG_INFO, TAG, "openStream()");

        // Create function list

        // Use shared_ptr to prevent use of a deleted callback.
        mDataCallback = std::make_shared < MyDataCallback > (this);
        mErrorCallback = std::make_shared < MyErrorCallback > (this);

        // Create an audio stream
        AudioStreamBuilder builder;
        builder.setChannelCount(mChannelCount);
        // we will resample source data to device rate, so take default sample rate
        builder.setDataCallback(mDataCallback);
        builder.setErrorCallback(mErrorCallback);
        builder.setPerformanceMode(PerformanceMode::LowLatency);
        builder.setSharingMode(SharingMode::Shared);
        builder.setSampleRateConversionQuality(SampleRateConversionQuality::Medium);

        Result result = builder.openStream(mAudioStream);
        if (result != Result::OK) {
            __android_log_print(
                    ANDROID_LOG_ERROR,
                    TAG,
                    "openStream failed. Error: %s", convertToText(result));
            return false;
        }

        // Reduce stream latency by setting the buffer size to a multiple of the burst size
        // Note: this will fail with ErrorUnimplemented if we are using a callback with OpenSL ES
        // See oboe::AudioStreamBuffered::setBufferSizeInFrames
        result = mAudioStream -> setBufferSizeInFrames(
                mAudioStream -> getFramesPerBurst() * kBufferSizeInBursts);
        if (result != Result::OK) {
            __android_log_print(
                    ANDROID_LOG_WARN,
                    TAG,
                    "setBufferSizeInFrames failed. Error: %s", convertToText(result));
        }

        mSampleRate = mAudioStream -> getSampleRate();

        return true;
    }

    // Just trying to open the stream if it is not already open.
    bool KaraokePlayerEngine::startStream() {
        int tryCount = 0;
        while (tryCount < 3) {
            bool wasOpenSuccessful = true;
            // Assume that apenStream() was called successfully before startStream() call.
            if (tryCount > 0) {
                usleep(20 * 1000); // Sleep between tries to give the system time to settle.
                wasOpenSuccessful = openStream(); // Try to open the stream again after the first try.
            }
            if (wasOpenSuccessful) {
                Result result = mAudioStream -> requestStart();
                if (result != Result::OK) {
                    __android_log_print(
                            ANDROID_LOG_ERROR,
                            TAG,
                            "requestStart failed. Error: %s", convertToText(result));
                    mAudioStream -> close();
                    mAudioStream.reset();
                } else {
                    return true;
                }
            }
            tryCount++;
        }

        return false;
    }

    void KaraokePlayerEngine::setupAudioStream(int32_t channelCount) {
        __android_log_print(ANDROID_LOG_INFO, TAG, "setupAudioStream()");
        mChannelCount = channelCount;
        openStream();
    }

    void KaraokePlayerEngine::teardownAudioStream() {
        __android_log_print(ANDROID_LOG_INFO, TAG, "teardownAudioStream()");
        this -> firstFrameHit = false;
        // tear down the player
        if (mAudioStream) {
            mAudioStream -> stop();
            mAudioStream -> close();
            mAudioStream.reset();
        }
    }

    void KaraokePlayerEngine::pauseStream() {
        __android_log_print(ANDROID_LOG_INFO, TAG, "pauseStream()");
        if (mAudioStream) {
            __android_log_print(ANDROID_LOG_INFO, TAG, "pauseStream() called");
            mAudioStream -> pause();
            mAudioStream -> flush(); // If available, clear pending buffers
            mIsStreamPaused = true;
            // More stuff needed to be done here
        }
    }

    void KaraokePlayerEngine::resumeStream() {
        __android_log_print(ANDROID_LOG_INFO, TAG, "resumeStream()");
        if (mAudioStream) {
            mAudioStream -> requestStart(); // Restart the stream
            mIsStreamPaused = false; // Reset paused flag
        }
    }

    void KaraokePlayerEngine::addSampleSource(SampleSource * source, SampleBuffer * buffer) {
        buffer -> resampleData(mSampleRate);
        mSampleBuffers.push_back(buffer);
        mSampleSources.push_back(source);
        mNumSampleBuffers++;
    }

    void KaraokePlayerEngine::unloadSampleData() {
        __android_log_print(ANDROID_LOG_INFO, TAG, "unloadSampleData()");
        resetAll();

        for (int32_t bufferIndex = 0; bufferIndex < mNumSampleBuffers; bufferIndex++) {
            delete mSampleSources[bufferIndex];
        }

        mSampleBuffers.clear();
        mSampleSources.clear();

        mNumSampleBuffers = 0;
    }

    void KaraokePlayerEngine::triggerDown(int32_t index) {
        if (index < mNumSampleBuffers) {
            // print timestamp of sample source
            struct timespec ts;
            // Get the time from CLOCK_MONOTONIC
            clock_gettime(CLOCK_MONOTONIC, & ts);
            // Convert to milliseconds
            long long currentTimeMillis = (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL);
            __android_log_print(ANDROID_LOG_INFO, TAG, "timestamp at triggerDown(): %lld", currentTimeMillis);
            __android_log_print(ANDROID_LOG_INFO, TAG, "triggerDown(%d)", index);

            mSampleSources[index] -> setPlayMode();
        }
    }

    void KaraokePlayerEngine::triggerUp(int32_t index) {
        this -> firstFrameHit = false;
        if (index < mNumSampleBuffers) {
            mSampleSources[index] -> setStopMode();
        }
    }

    void KaraokePlayerEngine::resetAll() {
        for (int32_t bufferIndex = 0; bufferIndex < mNumSampleBuffers; bufferIndex++) {
            mSampleSources[bufferIndex] -> setStopMode();
        }
    }

    void KaraokePlayerEngine::setPan(int index, float pan) {
        mSampleSources[index] -> setPan(pan);
    }

    float KaraokePlayerEngine::getPan(int index) {
        return mSampleSources[index] -> getPan();
    }

    void KaraokePlayerEngine::setGain(int index, float gain) {
        mSampleSources[index] -> setGain(gain);
    }

    float KaraokePlayerEngine::getGain(int index) {
        return mSampleSources[index] -> getGain();
    }
}