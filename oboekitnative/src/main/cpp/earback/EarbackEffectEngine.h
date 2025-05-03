#ifndef OBOE_LIVEEFFECTENGINE_H
#define OBOE_LIVEEFFECTENGINE_H

#include <jni.h>
#include <oboe/Oboe.h>
#include <string>
#include <thread>
#include "FullDuplexPass.h"
#include "FunctionList.h"
#include <variant>

class EarbackEffectEngine : public oboe::AudioStreamCallback {
public:
    EarbackEffectEngine();

    void setRecordingDeviceId(int32_t deviceId);
    void setPlaybackDeviceId(int32_t deviceId);
    static FunctionList<float*> effectsPipeline;

    /**
     * @param isOn
     * @return true if it succeeds
     */
    bool setEffectOn(bool isOn);

    /*
     * oboe::AudioStreamDataCallback interface implementation
     */
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *oboeStream,
                                          void *audioData, int32_t numFrames) override;

    /*
     * oboe::AudioStreamErrorCallback interface implementation
     */
    void onErrorBeforeClose(oboe::AudioStream *oboeStream, oboe::Result error) override;
    void onErrorAfterClose(oboe::AudioStream *oboeStream, oboe::Result error) override;

    bool setAudioApi(oboe::AudioApi);
    bool isAAudioRecommended(void);

    // FX Lab stuff
    std::variant<FunctionList<int16_t *>, FunctionList<float *>> functionList{
            std::in_place_type<FunctionList<int16_t *>>};

    void closeStreams();

private:
    bool              mIsEffectOn = false;
    int32_t           mRecordingDeviceId = oboe::kUnspecified;
    int32_t           mPlaybackDeviceId = oboe::kUnspecified;
    const oboe::AudioFormat mFormat = oboe::AudioFormat::Float; // for easier processing
    oboe::AudioApi    mAudioApi = oboe::AudioApi::AAudio;
    int32_t           mSampleRate = oboe::kUnspecified;
    const int32_t     mInputChannelCount = oboe::ChannelCount::Stereo;
    const int32_t     mOutputChannelCount = oboe::ChannelCount::Stereo;

    std::unique_ptr<FullDuplexPass> mDuplexStream;
    std::shared_ptr<oboe::AudioStream> mRecordingStream;
    std::shared_ptr<oboe::AudioStream> mPlayStream;

    oboe::Result beginStreams();

    void closeStream(std::shared_ptr<oboe::AudioStream> &stream);

    oboe::AudioStreamBuilder *setupCommonStreamParameters(
        oboe::AudioStreamBuilder *builder);
    oboe::AudioStreamBuilder *setupInStreamParameters(
        oboe::AudioStreamBuilder *builder, int32_t sampleRate);
    oboe::AudioStreamBuilder *setupOutStreamParameters(
        oboe::AudioStreamBuilder *builder);
    void warnIfNotLowLatency(std::shared_ptr<oboe::AudioStream> &stream);

};

#endif  // OBOE_LIVEEFFECTENGINE_H
