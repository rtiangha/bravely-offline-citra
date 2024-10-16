// Copyright 2024 Mandarine Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

package org.citra.citra_emu.utils

import android.app.Activity
import android.content.Context
import android.net.ConnectivityManager
import android.net.NetworkCapabilities
import android.net.wifi.WifiManager
import android.text.format.Formatter
import android.widget.Toast
import androidx.preference.PreferenceManager
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import java.net.InetAddress
import java.nio.ByteBuffer
import org.citra.citra_emu.NativeLibrary
import org.citra.citra_emu.R
import org.citra.citra_emu.databinding.DialogMultiplayerRoomBinding
import org.citra.citra_emu.ui.main.MainActivity

object NetPlayManager {
    fun showCreateRoomDialog(activity: Activity) {
        showRoomDialog(activity, isCreateRoom = true)
    }

    fun showJoinRoomDialog(activity: Activity) {
        showRoomDialog(activity, isCreateRoom = false)
    }

    private fun showRoomDialog(activity: Activity, isCreateRoom: Boolean) {
        val binding = DialogMultiplayerRoomBinding.inflate(activity.layoutInflater)
        val dialog = MaterialAlertDialogBuilder(activity)
            .setCancelable(true)
            .setView(binding.root)
            .show()

        // Set initial values and dialog title
        binding.textTitle.setText(
            if (isCreateRoom) R.string.multiplayer_create_room else R.string.multiplayer_join_room
        )
        binding.ipAddress.setText(
            if (isCreateRoom) getIpAddressByWifi(activity) else getRoomAddress(activity)
        )
        binding.ipPort.setText(getRoomPort(activity))
        binding.username.setText(getUsername(activity))

        binding.btnConfirm.setOnClickListener {
            val ipAddress = binding.ipAddress.text.toString()
            val username = binding.username.text.toString()
            val portStr = binding.ipPort.text.toString()

            if (!isInputValid(activity, ipAddress, username, portStr)) return@setOnClickListener

            val port = portStr.toInt()
            val resultCode = if (isCreateRoom) {
                netPlayCreateRoom(ipAddress, port, username)
            } else {
                netPlayJoinRoom(ipAddress, port, username)
            }

            handleRoomActionResult(activity, resultCode, ipAddress, username, portStr, dialog, isCreateRoom)
        }
    }

    private fun isInputValid(activity: Activity, ipAddress: String, username: String, portStr: String): Boolean {
        if (ipAddress.length < 7 || username.length < 5) {
            Toast.makeText(activity, R.string.multiplayer_input_invalid, Toast.LENGTH_LONG).show()
            return false
        }
        return try {
            portStr.toInt()
            true
        } catch (e: Exception) {
            Toast.makeText(activity, R.string.multiplayer_port_invalid, Toast.LENGTH_LONG).show()
            false
        }
    }

    private fun handleRoomActionResult(
        activity: Activity,
        resultCode: Int,
        ipAddress: String,
        username: String,
        portStr: String,
        dialog: androidx.appcompat.app.AlertDialog,
        isCreateRoom: Boolean
    ) {
        if (resultCode == NetPlayStatus.NO_ERROR) {
            setRoomAddress(activity, ipAddress)
            setUsername(activity, username)
            setRoomPort(activity, portStr)
            val successMessage = if (isCreateRoom) {
                R.string.multiplayer_create_room_success
            } else {
                R.string.multiplayer_join_room_success
            }
            Toast.makeText(activity, successMessage, Toast.LENGTH_LONG).show()
            dialog.dismiss()
        } else {
            val errorMessage = formatNetPlayStatus(activity, resultCode, "")
            Toast.makeText(activity, errorMessage, Toast.LENGTH_LONG).show()
        }
    }

    private fun getUsername(activity: Activity): String {
        val prefs = PreferenceManager.getDefaultSharedPreferences(activity)
        val defaultName = "Citra${(Math.random() * 100).toInt()}"
        return prefs.getString("NetPlayUsername", defaultName) ?: defaultName
    }

    private fun setUsername(activity: Activity, name: String) {
        PreferenceManager.getDefaultSharedPreferences(activity)
            .edit().putString("NetPlayUsername", name).apply()
    }

    private fun getRoomAddress(activity: Activity): String {
        val prefs = PreferenceManager.getDefaultSharedPreferences(activity)
        return prefs.getString("NetPlayRoomAddress", getIpAddressByWifi(activity)) ?: getIpAddressByWifi(activity)
    }

    private fun setRoomAddress(activity: Activity, address: String) {
        PreferenceManager.getDefaultSharedPreferences(activity)
            .edit().putString("NetPlayRoomAddress", address).apply()
    }

    private fun getRoomPort(activity: Activity): String {
        return PreferenceManager.getDefaultSharedPreferences(activity)
            .getString("NetPlayRoomPort", "24872") ?: "24872"
    }

    private fun setRoomPort(activity: Activity, port: String) {
        PreferenceManager.getDefaultSharedPreferences(activity)
            .edit().putString("NetPlayRoomPort", port).apply()
    }

    private external fun netPlayCreateRoom(ipAddress: String, port: Int, username: String): Int
    private external fun netPlayJoinRoom(ipAddress: String, port: Int, username: String): Int

    external fun netPlayRoomInfo(): Array<String>
    external fun netPlayIsJoined(): Boolean
    external fun netPlayIsHostedRoom(): Boolean
    external fun netPlaySendMessage(msg: String)
    external fun netPlayKickUser(username: String)
    external fun netPlayLeaveRoom()
    external fun netPlayGetConsoleId(): String

    fun addNetPlayMessage(type: Int, msg: String) {
        val emulationActivity = NativeLibrary.sEmulationActivity.get()
        val mainActivity = MainActivity.get()

        when {
            emulationActivity != null -> {
                emulationActivity.runOnUiThread {
                    emulationActivity.addNetPlayMessage(formatNetPlayStatus(emulationActivity, type, msg))
                }
            }
            mainActivity != null -> {
                mainActivity.runOnUiThread {
                    mainActivity.addNetPlayMessage(formatNetPlayStatus(mainActivity, type, msg))
                }
            }
        }
    }

    private fun formatNetPlayStatus(context: Context, type: Int, msg: String): String {
        return when (type) {
            NetPlayStatus.NETWORK_ERROR -> context.getString(R.string.multiplayer_network_error)
            NetPlayStatus.LOST_CONNECTION -> context.getString(R.string.multiplayer_lost_connection)
            NetPlayStatus.NAME_COLLISION -> context.getString(R.string.multiplayer_name_collision)
            NetPlayStatus.MAC_COLLISION -> context.getString(R.string.multiplayer_mac_collision)
            NetPlayStatus.CONSOLE_ID_COLLISION -> context.getString(R.string.multiplayer_console_id_collision)
            NetPlayStatus.WRONG_VERSION -> context.getString(R.string.multiplayer_wrong_version)
            NetPlayStatus.WRONG_PASSWORD -> context.getString(R.string.multiplayer_wrong_password)
            NetPlayStatus.COULD_NOT_CONNECT -> context.getString(R.string.multiplayer_could_not_connect)
            NetPlayStatus.ROOM_IS_FULL -> context.getString(R.string.multiplayer_room_is_full)
            NetPlayStatus.HOST_BANNED -> context.getString(R.string.multiplayer_host_banned)
            NetPlayStatus.PERMISSION_DENIED -> context.getString(R.string.multiplayer_permission_denied)
            NetPlayStatus.NO_SUCH_USER -> context.getString(R.string.multiplayer_no_such_user)
            NetPlayStatus.ALREADY_IN_ROOM -> context.getString(R.string.multiplayer_already_in_room)
            NetPlayStatus.CREATE_ROOM_ERROR -> context.getString(R.string.multiplayer_create_room_error)
            NetPlayStatus.HOST_KICKED -> context.getString(R.string.multiplayer_host_kicked)
            NetPlayStatus.UNKNOWN_ERROR -> context.getString(R.string.multiplayer_unknown_error)
            NetPlayStatus.ROOM_UNINITIALIZED -> context.getString(R.string.multiplayer_room_uninitialized)
            NetPlayStatus.ROOM_IDLE -> context.getString(R.string.multiplayer_room_idle)
            NetPlayStatus.ROOM_JOINING -> context.getString(R.string.multiplayer_room_joining)
            NetPlayStatus.ROOM_JOINED -> context.getString(R.string.multiplayer_room_joined)
            NetPlayStatus.ROOM_MODERATOR -> context.getString(R.string.multiplayer_room_moderator)
            NetPlayStatus.MEMBER_JOIN -> context.getString(R.string.multiplayer_member_join, msg)
            NetPlayStatus.MEMBER_LEAVE -> context.getString(R.string.multiplayer_member_leave, msg)
            NetPlayStatus.MEMBER_KICKED -> context.getString(R.string.multiplayer_member_kicked, msg)
            NetPlayStatus.MEMBER_BANNED -> context.getString(R.string.multiplayer_member_banned, msg)
            NetPlayStatus.ADDRESS_UNBANNED -> context.getString(R.string.multiplayer_address_unbanned)
            NetPlayStatus.CHAT_MESSAGE -> msg
            else -> ""
        }
    }

    private fun getIpAddressByWifi(activity: Activity): String {
        val connectivityManager = activity.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
        val network = connectivityManager.activeNetwork
        val networkCapabilities = connectivityManager.getNetworkCapabilities(network)
        val linkProperties = connectivityManager.getLinkProperties(network)

        val wifiInfo = networkCapabilities?.transportInfo as? android.net.wifi.WifiInfo
        val ipAddress = linkProperties?.linkAddresses?.firstOrNull()?.address

        return if (ipAddress == null) {
            "192.168.0.1"
        } else {
            ipAddress.hostAddress
    }
    }

    object NetPlayStatus {
        const val NO_ERROR = 0
        const val NETWORK_ERROR = 1
        const val LOST_CONNECTION = 2
        const val NAME_COLLISION = 3
        const val MAC_COLLISION = 4
        const val CONSOLE_ID_COLLISION = 5
        const val WRONG_VERSION = 6
        const val WRONG_PASSWORD = 7
        const val COULD_NOT_CONNECT = 8
        const val ROOM_IS_FULL = 9
        const val HOST_BANNED = 10
        const val PERMISSION_DENIED = 11
        const val NO_SUCH_USER = 12
        const val ALREADY_IN_ROOM = 13
        const val CREATE_ROOM_ERROR = 14
        const val HOST_KICKED = 15
        const val UNKNOWN_ERROR = 16
        const val ROOM_UNINITIALIZED = 17
        const val ROOM_IDLE = 18
        const val ROOM_JOINING = 19
        const val ROOM_JOINED = 20
        const val ROOM_MODERATOR = 21
        const val MEMBER_JOIN = 22
        const val MEMBER_LEAVE = 23
        const val MEMBER_KICKED = 24
        const val MEMBER_BANNED = 25
        const val ADDRESS_UNBANNED = 26
        const val CHAT_MESSAGE = 27
    }
}
