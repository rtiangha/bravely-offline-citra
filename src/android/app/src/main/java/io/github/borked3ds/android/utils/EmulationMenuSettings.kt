// Copyright 2023 Citra Emulator Project
// Copyright 2024 Borked3DS Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

package io.github.borked3ds.android.utils

import androidx.drawerlayout.widget.DrawerLayout
import androidx.preference.PreferenceManager
import io.github.borked3ds.android.Borked3DSApplication

object EmulationMenuSettings {
    private val preferences =
        PreferenceManager.getDefaultSharedPreferences(Borked3DSApplication.appContext)

    var joystickRelCenter: Boolean
        get() = preferences.getBoolean("EmulationMenuSettings_JoystickRelCenter", true)
        set(value) {
            preferences.edit()
                .putBoolean("EmulationMenuSettings_JoystickRelCenter", value)
                .apply()
        }
    var dpadSlide: Boolean
        get() = preferences.getBoolean("EmulationMenuSettings_DpadSlideEnable", true)
        set(value) {
            preferences.edit()
                .putBoolean("EmulationMenuSettings_DpadSlideEnable", value)
                .apply()
        }

    var showPerfStatsOvelray: Boolean
        get() = preferences.getBoolean("EmulationMenuSettings_showPerfStatsOvelray", false)
        set(value) {
            preferences.edit()
                .putBoolean("EmulationMenuSettings_showPerfStatsOvelray", value)
                .apply()
        }
    var hapticFeedback: Boolean
        get() = preferences.getBoolean("EmulationMenuSettings_HapticFeedback", true)
        set(value) {
            preferences.edit()
                .putBoolean("EmulationMenuSettings_HapticFeedback", value)
                .apply()
        }
    var swapScreens: Boolean
        get() = preferences.getBoolean("EmulationMenuSettings_SwapScreens", false)
        set(value) {
            preferences.edit()
                .putBoolean("EmulationMenuSettings_SwapScreens", value)
                .apply()
        }
    var showOverlay: Boolean
        get() = preferences.getBoolean("EmulationMenuSettings_ShowOverlay", true)
        set(value) {
            preferences.edit()
                .putBoolean("EmulationMenuSettings_ShowOverlay", value)
                .apply()
        }
    var drawerLockMode: Int
        get() = preferences.getInt(
            "EmulationMenuSettings_DrawerLockMode",
            DrawerLayout.LOCK_MODE_UNLOCKED
        )
        set(value) {
            preferences.edit()
                .putInt("EmulationMenuSettings_DrawerLockMode", value)
                .apply()
        }
}
