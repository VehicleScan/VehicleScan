package com.example.diagnostic_android_app

import android.car.Car
import android.car.hardware.CarPropertyValue
import android.car.hardware.property.CarPropertyManager
import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.ImageView
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import com.google.android.material.button.MaterialButton
import com.google.android.material.textview.MaterialTextView
import androidx.navigation.fragment.findNavController
import kotlinx.coroutines.launch

class UdsDetailFragment : Fragment(R.layout.fragment_uds_detail) {



    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {

        super.onViewCreated(view, savedInstanceState)



        val itemId = arguments?.getInt("itemId") ?: 1
        val item = UdsData.items.firstOrNull { it.id == itemId }
        item?.let {
            view.findViewById<MaterialTextView>(R.id.name)?.text = it.name
            view.findViewById<MaterialTextView>(R.id.details)?.text = it.details
            view.findViewById<ImageView>(R.id.sensor_image)?.setImageResource(it.iconRes)
            if (it.needsGarageSupport) {
                view.findViewById<MaterialTextView>(R.id.garage_support_status)?.text =
                    getString(R.string.needs_garage_support)
            } else {
                view.findViewById<MaterialTextView>(R.id.garage_support_status)?.visibility = View.GONE
            }


            view.findViewById<MaterialButton>(R.id.back_button)?.setOnClickListener {
                findNavController().navigate(R.id.action_uds_detail_to_uds_list)
            }
        } ?: run {
            view.findViewById<MaterialTextView>(R.id.name)?.text = "Item Not Found"
            view.findViewById<MaterialButton>(R.id.back_button)?.setOnClickListener {
                findNavController().navigate(R.id.action_uds_detail_to_uds_list)
            }
        }
    }
}