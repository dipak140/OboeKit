package `in`.reconv.oboekit

import android.os.Bundle
import android.widget.Button
import android.content.Intent
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Set app title
        supportActionBar?.title = "OboeKit Tester"

        // Buttons and their actions
        findViewById<Button>(R.id.btnRecording).setOnClickListener {
            startActivity(Intent(this, RecordingActivity::class.java))
        }

        findViewById<Button>(R.id.btnEarback).setOnClickListener {
            startActivity(Intent(this, EarbackActivity::class.java))
        }

        findViewById<Button>(R.id.btnPreviewPlayer).setOnClickListener {
            startActivity(Intent(this, PreviewPlayerActivity::class.java))
        }

        findViewById<Button>(R.id.btnLivePlayer).setOnClickListener {
            startActivity(Intent(this, LivePlayerActivity::class.java))
        }

    }
}