package `in`.reconv.oboekitnative

import android.util.Log
import java.io.File
import java.io.FileInputStream
import java.io.IOException

class KaraokePlayerNativeLib {

    external fun createPlayer(): Boolean

    companion object {
        val NUM_PLAY_CHANNELS: Int = 2
        init {
            System.loadLibrary("oboekitnative")
        }
        val TAG: String = "MusicPlayer"
    }

    fun loadWavFile(filePath: String, index: Int, pan: Float) {
        try {
            // Open the file using FileInputStream
            val file = File(filePath)
            val dataStream = FileInputStream(file)

            // Get the file size
            val dataLen = file.length().toInt()

            // Read the file data into a byte array
            val dataBytes = ByteArray(dataLen)
            dataStream.read(dataBytes, 0, dataLen)

            // Call your native function with the data
            loadWavAssetNative(dataBytes, index, pan)

            // Close the stream
            dataStream.close()
        } catch (ex: IOException) {
            Log.i(TAG, "IOException: $ex")
        }
    }

    fun setupAudioStream() {
        setupAudioStreamNative(NUM_PLAY_CHANNELS)
    }

    fun startAudioStream() {
        startAudioStreamNative()
    }

    fun teardownAudioStream() {
        teardownAudioStreamNative()
    }

    fun getDuration(mSampleRate: Int, mChannelCount: Int): Long{
        return getDurationNative(mSampleRate, mChannelCount)
    }

    fun unloadWavAssets() {
        unloadWavAssetsNative()
    }

    private external fun getDurationNative(mSampleRate: Int, mChannelCount: Int): Long
    private external fun setupAudioStreamNative(numChannels: Int)
    private external fun startAudioStreamNative()
    private external fun teardownAudioStreamNative()
    private external fun loadWavAssetNative(wavBytes: ByteArray, index: Int, pan: Float)
    private external fun unloadWavAssetsNative()
    external fun trigger(drumIndex: Int)
    external fun stopTrigger(drumIndex: Int)
    external fun setPan(index: Int, pan: Float)
    external fun getPan(index: Int): Float
    external fun setGain(index: Int, gain: Float)
    external fun getGain(index: Int): Float
    external fun getOutputReset() : Boolean
    external fun clearOutputReset()
    external fun restartStream()
    external fun pauseTrigger()
    external fun resumeTrigger()
    external fun seekToPosition(position: Long, mSampleRate : Int, mChannelCount: Int)
    external fun getPosition(index: Int, mSampleRate : Int, mChannelCount: Int): Long
    external fun getMusicPlayerTimeStamp(): Long
    external fun getMusicPlayerFramePosition(): Long
    external fun getPlayerAudioSessionId(): Int
    external fun deletePlayer()
}