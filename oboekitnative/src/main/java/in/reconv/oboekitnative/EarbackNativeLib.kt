package `in`.reconv.oboekitnative

import android.util.Log
import `in`.reconv.oboekitnative.datatype.EffectDescription
import `in`.reconv.oboekitnative.datatype.Effect

class EarbackNativeLib {
    val effectDescriptionMap: Map<String, EffectDescription>

    init {
        System.loadLibrary("oboekitnative")
        effectDescriptionMap = getEffects()
            .associateBy { it.name }
        Log.d("MAP", effectDescriptionMap.toString())
    }

    // Adds effect at end of effect list
    fun addEffect(effect: Effect) {
        Log.d("INTERFACE", String.format("Effect %s added", effect.name))
        addDefaultEffectNative(
            convertEffectToId(
                effect
            )
        )
    }

    // Enables effect at index
    fun enableEffectAt(turnOn: Boolean, index: Int) {
        Log.d("INTERFACE", String.format("Effect %b at index %d", turnOn, index))
        enableEffectNative(index, turnOn)
    }

    // Signals params were updated at index
    fun updateParamsAt(effect: Effect, index: Int) {
        Log.d(
            "INTERFACE",
            String.format(
                "Params were updated at index %d to %f",
                index,
                effect.paramValues[0]
            )
        )
        modifyEffectNative(
            convertEffectToId(
                effect
            ), index, effect.paramValues
        )
    }

    // Removes effect at index
    fun removeEffectAt(index: Int) {
        Log.d("INTERFACE", String.format("Effect was removed at index %d", index))
        removeEffectNative(index)
    }

    // Rotates existing effect from index to another
    fun rotateEffectAt(from: Int, to: Int) {
        Log.d("INTERFACE", String.format("Effect was rotated from %d to %d", from, to))
        rotateEffectNative(from, to)
    }

    fun enable(enable: Boolean) {
        Log.d("INTERFACE", "Enabling effects: $enable")
        enablePassthroughNative(enable)
    }

    // State of audio engine
    external fun createAudioEngine()

    external fun destroyAudioEngine()

    // These functions populate effectDescriptionMap
    private external fun getEffects(): Array<EffectDescription>

    // These functions mutate the function list
    // Adds effect at index
    private external fun addDefaultEffectNative(id: Int)

    private external fun removeEffectNative(index: Int)

    private external fun rotateEffectNative(from: Int, to: Int)

    private external fun modifyEffectNative(id: Int, index: Int, params: FloatArray)

    private external fun enableEffectNative(index: Int, enable: Boolean)

    private external fun enablePassthroughNative(enable: Boolean)

    // These are utility functions
    private fun convertEffectToId(effect: Effect): Int =
        effectDescriptionMap[effect.name]?.id ?: -1
}