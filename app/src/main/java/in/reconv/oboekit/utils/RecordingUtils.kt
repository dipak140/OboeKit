package `in`.reconv.oboekit.utils

import android.app.Activity
import android.os.Environment
import android.util.Log
import java.io.File
import java.io.IOException
import java.io.RandomAccessFile

object RecordingUtils {

    fun fixWavHeader(wavFile: File) {
        if (!wavFile.exists()) {
            Log.e("RecordingUtils", "WAV file not found: ${wavFile.absolutePath}")
            return
        }

        try {
            val fileSize = wavFile.length().toInt()
            val dataSize = fileSize - 44 // WAV header is 44 bytes

            RandomAccessFile(wavFile, "rw").use { raf ->
                // RIFF chunk descriptor
                raf.seek(4)
                raf.write(intToLittleEndian(fileSize - 8))

                // Data chunk size
                raf.seek(40)
                raf.write(intToLittleEndian(dataSize))
            }

            Log.d("RecordingUtils", "WAV header fixed: ${wavFile.absolutePath}, Size: $fileSize")
        } catch (e: IOException) {
            e.printStackTrace()
            Log.e("RecordingUtils", "Error fixing WAV header: ${e.message}")
        }
    }

    private fun intToLittleEndian(value: Int): ByteArray {
        return byteArrayOf(
            (value and 0xFF).toByte(),
            ((value shr 8) and 0xFF).toByte(),
            ((value shr 16) and 0xFF).toByte(),
            ((value shr 24) and 0xFF).toByte()
        )
    }

    fun getRecordingFilePath(activity: Activity, fileName: String): String {
        val externalDirectory = activity.getExternalFilesDir(Environment.DIRECTORY_MUSIC)
        val recordingFile = File(externalDirectory, fileName)
        return try {
            if (!recordingFile.exists()) {
                recordingFile.createNewFile()
            }
            recordingFile.absolutePath
        } catch (e: IOException) {
            e.printStackTrace()
            ""
        }
    }
}
