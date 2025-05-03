package `in`.reconv.oboekit

import android.os.Bundle
import android.os.Environment
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import `in`.reconv.oboekitnative.KaraokePlayerNativeLib
import java.io.File

class KaraokePlayerActivity : AppCompatActivity() {

    private lateinit var karaokePlayer: KaraokePlayerNativeLib
    private var isPlaying = false
    private var isPlayerInitialized = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_karaoke_player)

        supportActionBar?.title = "Karaoke Player"
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        val externalMusicDirectory = getExternalFilesDir(Environment.DIRECTORY_MUSIC)
        val music = File(externalMusicDirectory, "sample.wav")

        // Initialize the karaoke player
        karaokePlayer = KaraokePlayerNativeLib()
        isPlayerInitialized = karaokePlayer.createPlayer()

        if (isPlayerInitialized) {
            karaokePlayer.setupAudioStream()
            karaokePlayer.loadWavFile(music.absolutePath, 0, 0.0f)
        } else {
            Toast.makeText(this, "Failed to initialize karaoke player", Toast.LENGTH_SHORT).show()
        }

        // Set up button click listeners
        findViewById<Button>(R.id.btnPlay).setOnClickListener {
            if (isPlayerInitialized) {
                if (!isPlaying) {
                    karaokePlayer.startAudioStream()
                    karaokePlayer.trigger(0) // Assuming index 0 for the main track
                    isPlaying = true
                } else {
                    karaokePlayer.resumeTrigger()
                }
                Toast.makeText(this, "Playing", Toast.LENGTH_SHORT).show()
            }
        }

        findViewById<Button>(R.id.btnPause).setOnClickListener {
            if (isPlayerInitialized && isPlaying) {
                karaokePlayer.pauseTrigger()
                Toast.makeText(this, "Paused", Toast.LENGTH_SHORT).show()
            }
        }

        findViewById<Button>(R.id.btnStop).setOnClickListener {
            if (isPlayerInitialized) {
                if (isPlaying) {
                    karaokePlayer.stopTrigger(0) // Assuming index 0 for the main track
                    isPlaying = false
                    // Seek back to beginning
                    karaokePlayer.seekToPosition(0, 44100, KaraokePlayerNativeLib.NUM_PLAY_CHANNELS)
                    Toast.makeText(this, "Stopped", Toast.LENGTH_SHORT).show()
                }
            }
        }

        findViewById<Button>(R.id.btnBack).setOnClickListener {
            finish()
        }
    }

    override fun onSupportNavigateUp(): Boolean {
        finish()
        return true
    }

    override fun onResume() {
        super.onResume()
        if (isPlayerInitialized && karaokePlayer.getOutputReset()) {
            karaokePlayer.clearOutputReset()
            karaokePlayer.restartStream()
        }
    }

    override fun onPause() {
        super.onPause()
        if (isPlayerInitialized && isPlaying) {
            karaokePlayer.pauseTrigger()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        if (isPlayerInitialized) {
            if (isPlaying) {
                karaokePlayer.stopTrigger(0)
                isPlaying = false
            }
            karaokePlayer.unloadWavAssets()
            karaokePlayer.teardownAudioStream()
            karaokePlayer.deletePlayer()
        }
    }
}