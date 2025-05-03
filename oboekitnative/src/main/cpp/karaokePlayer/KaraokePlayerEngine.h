//
// Created by Dipak Sisodiya on 21/03/25.
//

#ifndef OBOEKIT_KARAOKEPLAYERENGINE_H
#define OBOEKIT_KARAOKEPLAYERENGINE_H

#include <vector>

#include <oboe/Oboe.h>
#include <player/OneShotSampleSource.h>
#include <player/SampleBuffer.h>
#include <array>
#include <algorithm>
#include <variant>
#include "player/AudioRingBuffer.h"

namespace iolib {
/**
 * A simple player for playing audio samples,
 * specifically for karaoke applications due to its low latency.
 */

    class KaraokePlayerEngine {
    public:
        KaraokePlayerEngine();

        void setupAudioStream(int32_t channelCount);
        void teardownAudioStream();

        bool openStream();
        bool startStream();
        bool  firstFrameHit = false;

        int getSampleRate() { return mSampleRate; }

        // Wave Sample Loading...
        /**
         * Adds the SampleSource/SampleBuffer pair to the list of source channels.
         * Transfers ownership of those objects so that they can be deleted/unloaded.
         * The indexes associated with each source channel is the order in which they
         * are added.
         */
        void addSampleSource(SampleSource* source, SampleBuffer* buffer);
        /**
         * Deallocates and deletes all added source/buffer (see addSampleSource()).
         */
        void unloadSampleData();

        void triggerDown(int32_t index);
        void triggerUp(int32_t index);

        void resetAll();

        bool getOutputReset() { return mOutputReset; }
        void clearOutputReset() { mOutputReset = false; }

        void setPan(int index, float pan);
        float getPan(int index);
        void setGain(int index, float gain);
        float getGain(int index);
        void pauseStream();
        void resumeStream();
        void seekTo(int64_t positionMillis, int mSampleRate, int mNumChannels);
        int64_t getPosition(int index, int mSampleRate, int mNumChannels);
        int64_t getDuration(int i, int channelCount);
        int64_t getFramePosition();
        int64_t getFrameTimeStamp();
        int64_t framePosition = 0;
        int64_t presentationTime = 0;
        int getAudioSessionId();

    private:
        class MyDataCallback : public oboe::AudioStreamDataCallback {
        public:
            MyDataCallback(KaraokePlayerEngine *parent) : mParent(parent) {}

            oboe::DataCallbackResult onAudioReady(
                    oboe::AudioStream *audioStream,
                    void *audioData,
                    int32_t numFrames) override;

        private:
            KaraokePlayerEngine *mParent;
        };
        class MyErrorCallback : public oboe::AudioStreamErrorCallback {
        public:
            MyErrorCallback(KaraokePlayerEngine *parent) : mParent(parent) {}

            virtual ~MyErrorCallback() {
            }

            void onErrorAfterClose(oboe::AudioStream *oboeStream, oboe::Result error) override;

        private:
            KaraokePlayerEngine *mParent;
        };
        // Oboe Audio Stream
        std::shared_ptr<oboe::AudioStream> mAudioStream;
        // Playback Audio attributes
        int32_t mChannelCount;
        int32_t mSampleRate;
        // Sample Data
        int32_t mNumSampleBuffers;
        std::vector<SampleBuffer*>  mSampleBuffers;
        std::vector<SampleSource*>  mSampleSources;
        bool    mOutputReset;
        std::shared_ptr<MyDataCallback> mDataCallback;
        std::shared_ptr<MyErrorCallback> mErrorCallback;
        bool mIsStreamPaused;
        };
    }

#endif //OBOEKIT_KARAOKEPLAYERENGINE_H
