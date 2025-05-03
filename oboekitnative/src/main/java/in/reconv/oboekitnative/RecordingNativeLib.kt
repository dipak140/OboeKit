package `in`.reconv.oboekitnative

class RecordingNativeLib {

    /**
     * A native method that is implemented by the 'oboekitnative' native library,
     * which is packaged with this application.
     */
    companion object {
        // Used to load the 'oboekitnative' library on application startup.
        init {
            System.loadLibrary("oboekitnative")
        }
    }

    // recording specific methods
    external fun startRecording(
        fullPathTofile: String?,
        inputPresetPreference: Int,
        startRecordingTime: Long
    )

    external fun stopRecording()
    external fun resumeRecording()
    external fun pauseRecording()
    external fun releaseRecordingEngine()
    external fun getRecorderTimeStamp(): Long
    external fun getRecorderFramePosition(): Long
    external fun getRecorderAudioSessionId(): Int

}