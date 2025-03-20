package `in`.reconv.oboekit

import android.content.pm.PackageManager
import android.media.MediaPlayer
import android.os.Bundle
import android.util.Log
import android.widget.Button
import android.widget.ImageButton
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import `in`.reconv.oboekitnative.RecordingNativeLib
import `in`.reconv.oboekit.utils.RecordingUtils
import java.io.File
import java.io.IOException
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

class RecordingActivity : AppCompatActivity() {
    var nativeRecorder: RecordingNativeLib = RecordingNativeLib()
    var isRecordingPaused: Boolean = false
    private var mediaPlayer: MediaPlayer? = null
    private lateinit var recordedFilePath: String
    private var isRecording = false

    private lateinit var btnStartRecording: ImageButton
    private lateinit var btnPlayRecording: Button
    private lateinit var btnStopRecording: ImageButton
    private lateinit var btnPauseResumeRecording: ImageButton
    private lateinit var earbackButton: ImageButton
    private lateinit var btnBack: ImageButton

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_recording)
        requestPermissions()

        supportActionBar?.title = "Oboekit Recording Test"
        supportActionBar?.setDisplayHomeAsUpEnabled(true)

        recordedFilePath = RecordingUtils.getRecordingFilePath(this, "recording.wav")

        initViews()
        setupClickListeners()
    }

    private fun initViews() {
        btnStartRecording = findViewById(R.id.btnStartRecording)
        btnPlayRecording = findViewById(R.id.btnPlayRecording)
        btnStopRecording = findViewById(R.id.btnStopRecording)
        btnPauseResumeRecording = findViewById(R.id.btnPauseResumeRecording)
        btnBack = findViewById(R.id.btnBack)
        earbackButton = findViewById(R.id.btnEarback)
    }

    private fun setupClickListeners() {
        btnStartRecording.setOnClickListener {
            startRecording(recordedFilePath, 9, System.currentTimeMillis())
        }

        btnPlayRecording.setOnClickListener {
            playRecording()
        }

        btnStopRecording.setOnClickListener {
            stopRecording()
        }

        btnPauseResumeRecording.setOnClickListener {
            togglePauseResumeRecording()
        }

        earbackButton.setOnClickListener {

        }

        btnBack.setOnClickListener {
            finish()
        }
    }

    private fun playRecording() {
        try {
            mediaPlayer?.release()
            mediaPlayer = MediaPlayer().apply {
                setDataSource(recordedFilePath)
                prepare()
                start()
            }

            Toast.makeText(this, "Playing merged audio", Toast.LENGTH_SHORT).show()

        } catch (e: IOException) {
            Log.e("TAG", "Error playing merged audio: ${e.message}")
            Toast.makeText(this, "Failed to play merged audio", Toast.LENGTH_SHORT).show()
        }
    }

    private fun startRecording(recordedFilePath: String, i: Int, currentTimeMillis: Long) {
        if (isRecording) {
            Toast.makeText(this, "Recording is already in progress", Toast.LENGTH_SHORT).show()
            return
        }
        isRecording = true
        lifecycleScope.launch(Dispatchers.IO) {
            nativeRecorder.startRecording(recordedFilePath, 9, System.currentTimeMillis())
        }
    }

    private fun stopRecording() {
        isRecording = false
        nativeRecorder.stopRecording()

        RecordingUtils.fixWavHeader(File(recordedFilePath))
    }

    private fun togglePauseResumeRecording() {
        if (isRecordingPaused) {
            nativeRecorder.resumeRecording()
            btnPauseResumeRecording.setImageResource(R.drawable.ic_pause_resume)
        } else {
            nativeRecorder.pauseRecording()
            btnPauseResumeRecording.setImageResource(R.drawable.ic_play)
        }
        isRecordingPaused = !isRecordingPaused
    }

    private fun requestPermissions() {
        val neededPermissions = listOf(
            android.Manifest.permission.RECORD_AUDIO,
            android.Manifest.permission.WRITE_EXTERNAL_STORAGE
        ).filter {
            ContextCompat.checkSelfPermission(this, it) == PackageManager.PERMISSION_DENIED
        }.toTypedArray()

        if (neededPermissions.isNotEmpty()) {
            registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) { permissions ->
                permissions.forEach { (permission, granted) ->
                    if (!granted) {
                        Toast.makeText(this, "Missing permission: $permission", Toast.LENGTH_SHORT).show()
                    }
                }
            }.launch(neededPermissions)
        }
    }

    override fun onBackPressed() {
        super.onBackPressed()
        stopRecording()
        if (mediaPlayer?.isPlaying == true) {
            mediaPlayer?.stop()
        }
        finish()
    }

    override fun onDestroy() {
        super.onDestroy()
        stopRecording()
        if (mediaPlayer?.isPlaying == true) {
            mediaPlayer?.stop()
        }
        mediaPlayer?.release()
    }

    override fun onSupportNavigateUp(): Boolean {
        finish()
        return true
    }
}
