#include <cassert>
#include "EarbackEffectEngine.h"
#include "../utils/logging_macros.h"
#include "../utils/effects/Effects.h"

FunctionList<float*> EarbackEffectEngine::effectsPipeline;

EarbackEffectEngine::EarbackEffectEngine() {
    assert(mOutputChannelCount == mInputChannelCount);
    beginStreams();
}

void EarbackEffectEngine::setRecordingDeviceId(int32_t deviceId) {
    mRecordingDeviceId = deviceId;
}

void EarbackEffectEngine::setPlaybackDeviceId(int32_t deviceId) {
    mPlaybackDeviceId = deviceId;
}

bool EarbackEffectEngine::isAAudioRecommended() {
    return oboe::AudioStreamBuilder::isAAudioRecommended();
}

bool EarbackEffectEngine::setAudioApi(oboe::AudioApi api) {
    if (mIsEffectOn) return false;
    mAudioApi = api;
    return true;
}

bool EarbackEffectEngine::setEffectOn(bool isOn) {
    bool success = true;
    if (isOn != mIsEffectOn) {
        if (isOn) {
            success = beginStreams() == oboe::Result::OK;
            if (success) {
                mIsEffectOn = isOn;
            }
        } else {
            closeStreams();
            mIsEffectOn = isOn;
       }
    }
    return success;
}

void EarbackEffectEngine::closeStreams() {
    /*
    * Note: The order of events is important here.
    * The playback stream must be closed before the recording stream. If the
    * recording stream were to be closed first the playback stream's
    * callback may attempt to read from the recording stream
    * which would cause the app to crash since the recording stream would be
    * null.
    */
    if (mDuplexStream) {
        mDuplexStream->stop();
        // Add a brief delay to ensure callbacks have completed
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Clear callback function to prevent access to resources we're about to free
    if (mDuplexStream) {
        mDuplexStream->effectFunction = nullptr;
    }

    closeStream(mPlayStream);
    closeStream(mRecordingStream);
    mDuplexStream.reset();
}

oboe::Result  EarbackEffectEngine::beginStreams() {
    // Note: The order of stream creation is important. We create the playback
    // stream first, then use properties from the playback stream
    // (e.g. sample rate) to create the recording stream. By matching the
    // properties we should get the lowest latency path
    oboe::AudioStreamBuilder inBuilder, outBuilder;
    setupOutStreamParameters(&outBuilder);
    oboe::Result result = outBuilder.openStream(mPlayStream);
    if (result != oboe::Result::OK) {
        LOGE("Failed to open output stream. Error %s", oboe::convertToText(result));
        mSampleRate = oboe::kUnspecified;
        return result;
    } else {
        // The input stream needs to run at the same sample rate as the output.
        mSampleRate = mPlayStream->getSampleRate();
    }
    if (mPlayStream->getFormat() == oboe::AudioFormat::Float) {
        functionList.emplace<FunctionList<float *>>();
    } else if (mPlayStream->getFormat() == oboe::AudioFormat::I16) {
        functionList.emplace<FunctionList<int16_t *>>();
    } else {
        closeStreams();
    }

    SAMPLE_RATE = mPlayStream->getSampleRate();
    warnIfNotLowLatency(mPlayStream);

    setupInStreamParameters(&inBuilder, mSampleRate);
    result = inBuilder.openStream(mRecordingStream);
    if (result != oboe::Result::OK) {
        LOGE("Failed to open input stream. Error %s", oboe::convertToText(result));
        closeStream(mPlayStream);
        return result;
    }
    warnIfNotLowLatency(mRecordingStream);

    mDuplexStream = std::make_unique<FullDuplexPass>();
    mDuplexStream->setSharedInputStream(mRecordingStream);
    mDuplexStream->setSharedOutputStream(mPlayStream);
    mDuplexStream->start();
    mDuplexStream->effectFunction = [this](float* buffer, int32_t numSamples) {
        std::get<FunctionList<float *>>(this->functionList)(buffer, buffer + numSamples);
    };

    return result;
}

/**
 * Sets the stream parameters which are specific to recording,
 * including the sample rate which is determined from the
 * playback stream.
 *
 * @param builder The recording stream builder
 * @param sampleRate The desired sample rate of the recording stream
 */
oboe::AudioStreamBuilder *EarbackEffectEngine::setupInStreamParameters(
    oboe::AudioStreamBuilder *builder, int32_t sampleRate) {
    // This sample uses blocking read() because we don't specify a callback
    builder->setDeviceId(mRecordingDeviceId)
        ->setDirection(oboe::Direction::Input)
        ->setSampleRate(sampleRate)
        ->setChannelCount(mInputChannelCount);
    return setupCommonStreamParameters(builder);
}

/**
 * Sets the stream parameters which are specific to playback, including device
 * id and the dataCallback function, which must be set for low latency
 * playback.
 * @param builder The playback stream builder
 */
oboe::AudioStreamBuilder *EarbackEffectEngine::setupOutStreamParameters(
    oboe::AudioStreamBuilder *builder) {
    builder->setDataCallback(this)
        ->setErrorCallback(this)
        ->setDeviceId(mPlaybackDeviceId)
        ->setDirection(oboe::Direction::Output)
        ->setChannelCount(mOutputChannelCount);

    return setupCommonStreamParameters(builder);
}

/**
 * Set the stream parameters which are common to both recording and playback
 * streams.
 * @param builder The playback or recording stream builder
 */
oboe::AudioStreamBuilder *EarbackEffectEngine::setupCommonStreamParameters(
    oboe::AudioStreamBuilder *builder) {
    // We request EXCLUSIVE mode since this will give us the lowest possible
    // latency.
    // If EXCLUSIVE mode isn't available the builder will fall back to SHARED
    // mode.
    builder->setAudioApi(mAudioApi)
        ->setFormat(mFormat)
        ->setFormatConversionAllowed(true)
        ->setSharingMode(oboe::SharingMode::Exclusive)
        ->setPerformanceMode(oboe::PerformanceMode::LowLatency);
    return builder;
}

/**
 * Close the stream. AudioStream::close() is a blocking call so
 * the application does not need to add synchronization between
 * onAudioReady() function and the thread calling close().
 * [the closing thread is the UI thread in this sample].
 * @param stream the stream to close
 */
void EarbackEffectEngine::closeStream(std::shared_ptr<oboe::AudioStream> &stream) {
    if (stream) {
        oboe::Result result = stream->stop();
        if (result != oboe::Result::OK) {
            LOGW("Error stopping stream: %s", oboe::convertToText(result));
        }
        result = stream->close();
        if (result != oboe::Result::OK) {
            LOGE("Error closing stream: %s", oboe::convertToText(result));
        } else {
            LOGW("Successfully closed streams");
        }
        stream.reset();
    }
}

/**
 * Warn in logcat if non-low latency stream is created
 * @param stream: newly created stream
 *
 */
void EarbackEffectEngine::warnIfNotLowLatency(std::shared_ptr<oboe::AudioStream> &stream) {
    if (stream->getPerformanceMode() != oboe::PerformanceMode::LowLatency) {
        LOGW(
            "Stream is NOT low latency."
            "Check your requested format, sample rate and channel count");
    }
}

/**
 * Handles playback stream's audio request. In this sample, we simply block-read
 * from the record stream for the required samples.
 *
 * @param oboeStream: the playback stream that requesting additional samples
 * @param audioData:  the buffer to load audio samples for playback stream
 * @param numFrames:  number of frames to load to audioData buffer
 * @return: DataCallbackResult::Continue.
 */
oboe::DataCallbackResult EarbackEffectEngine::onAudioReady(
    oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {
    if (!mDuplexStream) {
        // Fill with zeros to prevent noise
        memset(audioData, 0, numFrames * oboeStream->getChannelCount() *
                             oboeStream->getBytesPerFrame());
        return oboe::DataCallbackResult::Stop;
    }
    return mDuplexStream->onAudioReady(oboeStream, audioData, numFrames);
}

/**
 * Oboe notifies the application for "about to close the stream".
 *
 * @param oboeStream: the stream to close
 * @param error: oboe's reason for closing the stream
 */
void EarbackEffectEngine::onErrorBeforeClose(oboe::AudioStream *oboeStream,
                                             oboe::Result error) {
    LOGE("%s stream Error before close: %s",
         oboe::convertToText(oboeStream->getDirection()),
         oboe::convertToText(error));
}

/**
 * Oboe notifies application that "the stream is closed"
 *
 * @param oboeStream
 * @param error
 */
void EarbackEffectEngine::onErrorAfterClose(oboe::AudioStream *oboeStream,
                                            oboe::Result error) {
    LOGE("%s stream Error after close: %s",
         oboe::convertToText(oboeStream->getDirection()),
         oboe::convertToText(error));

    closeStreams();

    // Restart the stream if the error is a disconnect.
    if (error == oboe::Result::ErrorDisconnected) {
        LOGI("Restarting AudioStream");
        beginStreams();
    }
}
