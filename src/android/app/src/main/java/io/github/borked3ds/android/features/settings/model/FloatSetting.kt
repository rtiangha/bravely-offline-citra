// Copyright 2023 Citra Emulator Project
// Copyright 2024 Borked3DS Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

package io.github.borked3ds.android.features.settings.model

enum class FloatSetting(
    override val key: String,
    override val section: String,
    override val defaultValue: Float
) : AbstractFloatSetting {
    BG_BLUE("bg_blue", Settings.SECTION_RENDERER, 0.0f),
    BG_GREEN("bg_green", Settings.SECTION_RENDERER, 0.0f),
    BG_RED("bg_red", Settings.SECTION_RENDERER, 0.0f),
    LARGE_SCREEN_PROPORTION("large_screen_proportion", Settings.SECTION_LAYOUT, 2.25f),
    SECOND_SCREEN_OPACITY("custom_second_layer_opacity", Settings.SECTION_LAYOUT, 100f),
    EMPTY_SETTING("", "", 0.0f);

    override var float: Float = defaultValue

    override val valueAsString: String
        get() = float.toString()

    override val isRuntimeEditable: Boolean
        get() {
            for (setting in NOT_RUNTIME_EDITABLE) {
                if (setting == this) {
                    return false
                }
            }
            return true
        }

    companion object {
        private val NOT_RUNTIME_EDITABLE = emptyList<FloatSetting>()

        fun from(key: String): FloatSetting? = FloatSetting.entries.firstOrNull { it.key == key }

        fun clear() = FloatSetting.entries.forEach { it.float = it.defaultValue }
    }
}
