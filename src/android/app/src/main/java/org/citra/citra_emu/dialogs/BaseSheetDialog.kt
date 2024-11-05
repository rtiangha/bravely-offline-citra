// Copyright 2024 Mandarine Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

package org.citra.citra_emu.dialogs

import android.content.Context
import android.view.LayoutInflater
import android.view.View
import android.widget.LinearLayout
import com.google.android.material.bottomsheet.BottomSheetDialog
import org.citra.citra_emu.databinding.DialogBottomSheetBinding
import org.citra.citra_emu.utils.CompatUtils

// TODO: Fix the dialog to resize when the keyboard is shown

open class BaseSheetDialog(context: Context) : BottomSheetDialog(CompatUtils.findActivity(context)) {
    private val contentView: LinearLayout
    private val binding = DialogBottomSheetBinding.inflate(layoutInflater)

    init {
        val width = CompatUtils.findActivity(context).window.decorView.measuredWidth
        val height = CompatUtils.findActivity(context).window.decorView.measuredHeight
        val heightScale = 0.87f // What percentage of the screen's height to use up

        behavior.peekHeight = (height * heightScale).toInt()
        behavior.maxHeight = (height * heightScale).toInt()
        behavior.maxWidth = width

        super.setContentView(binding.root)
        contentView = binding.content
    }

    override fun setContentView(view: View) {
        contentView.removeAllViews()
        contentView.addView(view)
    }

    override fun setContentView(layoutResID: Int) {
        setContentView(LayoutInflater.from(context).inflate(layoutResID, null, false))
    }

    override fun <T : View?> findViewById(id: Int): T {
        return contentView.findViewById<T>(id)!!
    }
}
