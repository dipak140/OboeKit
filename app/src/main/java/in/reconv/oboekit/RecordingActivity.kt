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
import `in`.reconv.oboekitnative.EarbackNativeLib
import `in`.reconv.oboekitnative.KaraokePlayerNativeLib
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

class RecordingActivity : AppCompatActivity() {
    var nativeRecorder: RecordingNativeLib = RecordingNativeLib()
    var earbackNativeLib: EarbackNativeLib = EarbackNativeLib()
    var karaokePlayer: KaraokePlayerNativeLib = KaraokePlayerNativeLib()

    private var isRecordingPaused: Boolean = false
    private var isEarbackEnabled: Boolean = false
    private var isKaraokePlayerInitialized = false
    private var isKaraokePlaying = false

    private var mediaPlayer: MediaPlayer? = null
    private lateinit var recordedFilePath: String
    private var isRecording = false

    private lateinit var btnStartRecording: ImageButton
    private lateinit var btnPlayRecording: Button
    private lateinit var btnStopRecording: ImageButton
    private lateinit var btnPauseResumeRecording: ImageButton
    private lateinit var earbackButton: ImageButton
    private lateinit var btnBack: ImageButton
    private lateinit var btnPlayMusic: Button
    private lateinit var btnPauseMusic: Button
    private lateinit var btnStopMusic: Button

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_recording)
        requestPermissions()
        earbackNativeLib.createAudioEngine()
        earbackNativeLib.enable(false)
        isKaraokePlayerInitialized = karaokePlayer.createPlayer()
        if (isKaraokePlayerInitialized) {
            karaokePlayer.setupAudioStream()
            val musicFile = File(getExternalFilesDir(android.os.Environment.DIRECTORY_MUSIC), "sample.wav")
            karaokePlayer.loadWavFile(musicFile.absolutePath, 0, 0.0f)
        } else {
            Toast.makeText(this, "Failed to init Karaoke Player", Toast.LENGTH_SHORT).show()
        }
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
        btnPlayMusic = findViewById(R.id.btnPlay)
        btnPauseMusic = findViewById(R.id.btnPause)
        btnStopMusic = findViewById(R.id.btnStop)
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
            if(isEarbackEnabled){
                earbackNativeLib.enable(false)
            }else {
                earbackNativeLib.enable(true)
            }
            isEarbackEnabled = !isEarbackEnabled
        }

        btnBack.setOnClickListener {
            finish()
        }

        btnPlayMusic.setOnClickListener {
            if (isKaraokePlayerInitialized) {
                if (!isKaraokePlaying) {
                    karaokePlayer.startAudioStream()
                    karaokePlayer.trigger(0)
                    isKaraokePlaying = true
                    Toast.makeText(this, "Playing Karaoke", Toast.LENGTH_SHORT).show()
                } else {
                    karaokePlayer.resumeTrigger()
                }
            }
        }

        btnPauseMusic.setOnClickListener {
            if (isKaraokePlayerInitialized && isKaraokePlaying) {
                karaokePlayer.pauseTrigger()
                Toast.makeText(this, "Paused", Toast.LENGTH_SHORT).show()
            }
        }

        btnStopMusic.setOnClickListener {
            if (isKaraokePlayerInitialized && isKaraokePlaying) {
                karaokePlayer.stopTrigger(0)
                isKaraokePlaying = false
                karaokePlayer.seekToPosition(0, 44100, KaraokePlayerNativeLib.NUM_PLAY_CHANNELS)
                Toast.makeText(this, "Stopped", Toast.LENGTH_SHORT).show()
            }
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
        earbackNativeLib.enable(false)
        earbackNativeLib.destroyAudioEngine()
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
        earbackNativeLib.enable(false)
        earbackNativeLib.destroyAudioEngine()
        finish()
    }

    override fun onDestroy() {
        super.onDestroy()
        stopRecording()
        if (mediaPlayer?.isPlaying == true) {
            mediaPlayer?.stop()
        }
        mediaPlayer?.release()
        earbackNativeLib.enable(false)
        earbackNativeLib.destroyAudioEngine()
        if (isKaraokePlayerInitialized) {
            if (isKaraokePlaying) {
                karaokePlayer.stopTrigger(0)
                isKaraokePlaying = false
            }
            karaokePlayer.unloadWavAssets()
            karaokePlayer.teardownAudioStream()
            karaokePlayer.deletePlayer()
        }
    }

    override fun onPause() {
        super.onPause()
        if (isKaraokePlayerInitialized && isKaraokePlaying) {
            karaokePlayer.pauseTrigger()
        }
    }

    override fun onSupportNavigateUp(): Boolean {
        finish()
        return true
    }
}
