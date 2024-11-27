// Copyright 2023 Citra Emulator Project
// Copyright 2024 Borked3DS Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

package io.github.borked3ds.android.features.settings.ui

import android.os.Bundle
import android.text.TextUtils
import io.github.borked3ds.android.NativeLibrary
import io.github.borked3ds.android.features.settings.model.BooleanSetting
import io.github.borked3ds.android.features.settings.model.Settings
import io.github.borked3ds.android.utils.DirectoryInitialization
import io.github.borked3ds.android.utils.FileUtil
import io.github.borked3ds.android.utils.Log
import io.github.borked3ds.android.utils.PermissionsHandler
import io.github.borked3ds.android.utils.PermissionsHandler.hasWriteAccess
import io.github.borked3ds.android.utils.SystemSaveGame

class SettingsActivityPresenter(private val activityView: SettingsActivityView) {
    val settings: Settings get() = activityView.settings

    private var shouldSave = false
    private lateinit var menuTag: String
    private lateinit var gameId: String

    fun onCreate(savedInstanceState: Bundle?, menuTag: String, gameId: String) {
        this.menuTag = menuTag
        this.gameId = gameId
        if (savedInstanceState != null) {
            shouldSave = savedInstanceState.getBoolean(KEY_SHOULD_SAVE)
        }
    }

    fun onStart() {
        SystemSaveGame.load()
        prepareDirectoriesIfNeeded()
    }

    private fun loadSettingsUI() {
        if (!settings.isLoaded) {
            if (!TextUtils.isEmpty(gameId)) {
                settings.loadSettings(gameId, activityView)
            } else {
                settings.loadSettings(activityView)
            }
        }
        activityView.showSettingsFragment(menuTag, false, gameId)
        activityView.onSettingsFileLoaded()
    }

    private fun prepareDirectoriesIfNeeded() {
        if (!DirectoryInitialization.areBorked3DSDirectoriesReady()) {
            DirectoryInitialization.start()
        }
        loadSettingsUI()
    }

    private fun hideImages() {
        val dataPath = PermissionsHandler.borked3dsDirectory.toString()
        val nomedia = FileUtil.createFile(dataPath, ".nomedia")
        if (!BooleanSetting.HIDE_IMAGES.boolean) {
            Log.debug("[SettingsActivityPresenter]: Trying to delete .nomedia in $dataPath")
            nomedia?.delete()
        }
    }

    fun onStop(finishing: Boolean) {
        if (finishing && shouldSave) {
            Log.debug("[SettingsActivity] Settings activity stopping. Saving settings to INI...")
            settings.saveSettings(activityView)
            SystemSaveGame.save()
            hideImages()
            //added to ensure that layout changes take effect as soon as settings window closes
            NativeLibrary.reloadSettings()
            NativeLibrary.updateFramebuffer(NativeLibrary.isPortraitMode)
        }
        NativeLibrary.reloadSettings()
    }

    fun onSettingChanged() {
        shouldSave = true
    }

    fun onSettingsReset() {
        shouldSave = false
    }

    fun saveState(outState: Bundle) {
        outState.putBoolean(KEY_SHOULD_SAVE, shouldSave)
    }

    companion object {
        private const val KEY_SHOULD_SAVE = "should_save"
    }
}
