package io.github.borked3ds.android.utils

import android.content.Context
import android.net.Uri
import androidx.preference.PreferenceManager
import io.github.borked3ds.android.Borked3DSApplication

interface SearchLocationHelper {
    companion object {

        /**
         * Returns the list of selected search locations.
         * @return A list of URIs of selected search locations
         */
        fun getSearchLocations(context: Context): List<Uri> {
            val preferences =
                PreferenceManager.getDefaultSharedPreferences(Borked3DSApplication.appContext)
            val locations = preferences.getString(GameHelper.KEY_GAME_PATH, "").orEmpty().split("|")
            val urisList = mutableListOf<Uri>()

            locations.forEach {
                if (it.isNotEmpty()) {
                    urisList.add(Uri.parse(it))
                }
            }
            return urisList
        }

        /**
         * Adds the given search location to the emulation settings
         * @param uri The URI of the selected search location
         * @return The exit status of the installation process
         */
        fun addLocation(context: Context, uri: Uri): SearchLocationResult {
            val locations = getSearchLocations(context)
            val preferences =
                PreferenceManager.getDefaultSharedPreferences(Borked3DSApplication.appContext)

            if (locations.contains(uri)) {
                return SearchLocationResult.AlreadyAdded
            } else {
                val newLocations = buildString {
                    append(locations.joinToString(separator = "|") { it.toString() })
                    if (locations.isNotEmpty()) append("|")
                    append(uri.toString())
                }

                preferences.edit()
                    .putString(GameHelper.KEY_GAME_PATH, newLocations)
                    .apply()
            }
            return SearchLocationResult.Success
        }

        /**
         * Deletes the given URI from the emulation settings
         * @param uri The URI to remove from the list
         * @return The exit status of the removal process
         */
        fun deleteLocation(context: Context, uri: Uri): SearchLocationResult {
            val locations = getSearchLocations(context)
            val preferences =
                PreferenceManager.getDefaultSharedPreferences(Borked3DSApplication.appContext)

            val newValue = locations.filterNot { it.toString() == uri.toString() }
                .joinToString(separator = "|") { it.toString() }

            preferences.edit()
                .putString(GameHelper.KEY_GAME_PATH, newValue)
                .apply()

            return SearchLocationResult.Deleted
        }
    }
}

enum class SearchLocationResult {
    Success,
    AlreadyAdded,
    Deleted
}
