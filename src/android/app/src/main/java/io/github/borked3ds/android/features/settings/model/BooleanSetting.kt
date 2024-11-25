// Copyright 2023 Citra Emulator Project
// Copyright 2024 Borked3DS Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

package io.github.borked3ds.android.features.settings.model

enum class BooleanSetting(
    override val key: String,
    override val section: String,
    override val defaultValue: Boolean
) : AbstractBooleanSetting {
    EXPAND_TO_CUTOUT_AREA("expand_to_cutout_area", Settings.SECTION_LAYOUT, false),
    SPIRV_SHADER_GEN("spirv_shader_gen", Settings.SECTION_RENDERER, true),
    USE_SAMPLE_SHADING("use_sample_shading", Settings.SECTION_RENDERER, false),
    GEOMETRY_SHADER("geometry_shader", Settings.SECTION_RENDERER, false),
    SPIRV_OUTPUT_VALIDATION("spirv_output_validation", Settings.SECTION_RENDERER, false),
    SPIRV_OUTPUT_LEGALIZATION("spirv_output_legalization", Settings.SECTION_RENDERER, false),
    ASYNC_SHADERS("async_shader_compilation", Settings.SECTION_RENDERER, false),
    PRELOAD_TEXTURES("preload_textures", Settings.SECTION_RENDERER, false),
    ASYNC_PRESENTATION("async_presentation", Settings.SECTION_RENDERER, true),
    SHADER_JIT("use_shader_jit", Settings.SECTION_RENDERER, true),
    CORE_DOWNCOUNT_HACK("core_downcount_hack", Settings.SECTION_RENDERER, false),
    PLUGIN_LOADER("plugin_loader", Settings.SECTION_SYSTEM, false),
    ALLOW_PLUGIN_LOADER("allow_plugin_loader", Settings.SECTION_SYSTEM, true),
    SWAP_SCREEN("swap_screen", Settings.SECTION_LAYOUT, false),
    GDB_STUB("use_gdbstub", Settings.SECTION_DEBUG, false),
    INSTANT_DEBUG_LOG("instant_debug_log", Settings.SECTION_DEBUG, false),
    DUMP_COMMAND_BUFFERS("dump_command_buffers", Settings.SECTION_DEBUG, false),
    USE_VIRTUAL_SD("use_virtual_sd", Settings.SECTION_DATA_STORAGE, true),
    RELAXED_PRECISION_DECORATORS("relaxed_precision_decorators", Settings.SECTION_RENDERER, false),
    CUSTOM_LAYOUT("custom_layout", Settings.SECTION_LAYOUT, false),
    ADRENO_GPU_BOOST("adreno_gpu_boost", Settings.SECTION_RENDERER, false),
    SWAP_EYES_3D("swap_eyes_3d", Settings.SECTION_RENDERER, false);

    override var boolean: Boolean = defaultValue

    override val valueAsString: String
        get() = boolean.toString()

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
        private val NOT_RUNTIME_EDITABLE = listOf(
            PLUGIN_LOADER,
            ALLOW_PLUGIN_LOADER,
            RELAXED_PRECISION_DECORATORS,
            ADRENO_GPU_BOOST,
            ASYNC_SHADERS
        )

        fun from(key: String): BooleanSetting? =
            BooleanSetting.entries.firstOrNull { it.key == key }

        fun clear() = BooleanSetting.entries.forEach { it.boolean = it.defaultValue }
    }
}
