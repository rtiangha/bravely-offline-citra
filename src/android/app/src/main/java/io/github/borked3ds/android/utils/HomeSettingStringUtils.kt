
package io.github.borked3ds.android.utils

sealed class HomeSettingStringUtils {
    data class Text(val value: String) : HomeSettingStringUtils()
    data class ResId(val id: Int) : HomeSettingStringUtils()
}
