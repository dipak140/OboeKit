package `in`.reconv.oboekitnative

class NativeLib {

    /**
     * A native method that is implemented by the 'oboekitnative' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {
        // Used to load the 'oboekitnative' library on application startup.
        init {
            System.loadLibrary("oboekitnative")
        }
    }
}