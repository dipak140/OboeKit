package `in`.reconv.oboekit

import android.os.Bundle
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity

class PreviewPlayerActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_preview_player)

        supportActionBar?.title = "Preview Player"
        supportActionBar?.setDisplayHomeAsUpEnabled(true)

        findViewById<Button>(R.id.btnBack).setOnClickListener {
            finish()
        }
    }

    override fun onSupportNavigateUp(): Boolean {
        finish()
        return true
    }
}
