package `in`.reconv.oboekit

import android.Manifest
import android.content.ContentValues
import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import `in`.reconv.oboekitnative.EarbackNativeLib
import `in`.reconv.oboekitnative.datatype.Effect

class EarbackActivity : AppCompatActivity() {
    private lateinit var earbackNativeLib: EarbackNativeLib
    private val MY_PERMISSIONS_RECORD_AUDIO = 17

    private var isEarbackEnabled = false
    private var areEffectsEnabled = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        earbackNativeLib = EarbackNativeLib()

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO)
            != PackageManager.PERMISSION_GRANTED
        ) {
            ActivityCompat.requestPermissions(
                this,
                arrayOf(Manifest.permission.RECORD_AUDIO),
                MY_PERMISSIONS_RECORD_AUDIO
            )
        }

        setContentView(R.layout.activity_earback)
        supportActionBar?.title = "Earback"
        supportActionBar?.setDisplayHomeAsUpEnabled(true)

        setupClickListeners()
    }

    override fun onResume() {
        super.onResume()
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO)
            == PackageManager.PERMISSION_GRANTED
        ) {
            earbackNativeLib.createAudioEngine()
            earbackNativeLib.enable(false)
        }
    }

    private fun setupClickListeners() {
        findViewById<Button>(R.id.btnBack).setOnClickListener {
            clearEffects()
            earbackNativeLib.enable(false)
            earbackNativeLib.destroyAudioEngine()
            finish()
        }

        findViewById<Button>(R.id.btnToggleEarback).setOnClickListener {
            toggleEarback()
        }

        findViewById<Button>(R.id.btnToggleEffects).setOnClickListener {
            toggleEffects()
        }
    }

    private fun toggleEarback() {
        isEarbackEnabled = !isEarbackEnabled
        earbackNativeLib.enable(isEarbackEnabled)

        val btnEarback = findViewById<Button>(R.id.btnToggleEarback)
        btnEarback.text = if (isEarbackEnabled) "Disable Earback" else "Enable Earback"

        Log.d("EarbackActivity", "Earback is now: ${if (isEarbackEnabled) "Enabled" else "Disabled"}")
    }

    private fun toggleEffects() {
        areEffectsEnabled = !areEffectsEnabled

        if (areEffectsEnabled) {
            prepopulateEffects()  // Adds effects
        } else {
            clearEffects()  // Removes all effects
        }

        val btnEffects = findViewById<Button>(R.id.btnToggleEffects)
        btnEffects.text = if (areEffectsEnabled) "Disable Effects" else "Enable Effects"

        Log.d("EarbackActivity", "Effects are now: ${if (areEffectsEnabled) "Enabled" else "Disabled"}")
    }

    private fun prepopulateEffects() {
        val effectsToAdd = listOf("Echo")
        effectsToAdd.forEach { effectName ->
            val effectDescription = earbackNativeLib.effectDescriptionMap[effectName]
            if (effectDescription != null) {
                val toAdd = Effect(effectDescription)
                earbackNativeLib.addEffect(toAdd)
                Log.d(ContentValues.TAG, "Prepopulated effect: $effectName")
            } else {
                Log.w(ContentValues.TAG, "Effect not found: $effectName")
            }
        }
    }

    private fun clearEffects() {
        // Assuming you have a way to track added effects, remove them
        for (i in 0 until earbackNativeLib.effectDescriptionMap.size) {
            earbackNativeLib.removeEffectAt(i)
        }
        Log.d("EarbackActivity", "All effects removed")
    }

    override fun onBackPressed() {
        super.onBackPressed()
        clearEffects()
        earbackNativeLib.enable(false)
        earbackNativeLib.destroyAudioEngine()
    }

    override fun onDestroy() {
        super.onDestroy()
        clearEffects()
        earbackNativeLib.enable(false)
        earbackNativeLib.destroyAudioEngine()
    }

    override fun onSupportNavigateUp(): Boolean {
        finish()
        return true
    }
}
