package com.example.diagnostic_android_app

import android.car.Car
import android.car.hardware.CarPropertyValue
import android.car.hardware.property.CarPropertyManager
import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.Button
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import androidx.navigation.fragment.findNavController
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import kotlinx.coroutines.launch

class UdsListFragment : Fragment(R.layout.fragment_uds_list) {

    private lateinit var clear_btn: Button
    private lateinit var send_sms_btn: Button
    private lateinit var send_report_btn: Button

    private lateinit var wifiSender: WifiSender

    val VENDOR_EXTENSION_STRING_DTC_PROPERTY:Int = 0x21100109

    val VENDOR_EXTENSION_INIT_UDS_PROPERTY:Int = 0x2140010A


    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        val recycler:RecyclerView = view.findViewById(R.id.uds_list)
        wifiSender = WifiSender(requireContext())
        val car = Car.createCar(context)

        val carPropertyManager = car!!.getCarManager(Car.PROPERTY_SERVICE) as CarPropertyManager

        carPropertyManager.registerCallback(object : CarPropertyManager.CarPropertyEventCallback{
            override fun onChangeEvent(p0: CarPropertyValue<*>?) {
                for (i in 1..8) {
                    UdsData.updateExist(i,1)
                }
                // show all list items (dtcs)
                recycler?.apply {
                    layoutManager = LinearLayoutManager(requireContext())
                    adapter = UdsAdapter(UdsData.items.filter { it.exist == 1 }) { itemId ->
                        findNavController().navigate(
                            R.id.action_uds_list_to_uds_detail,
                            Bundle().apply { putInt("itemId", itemId) }
                        )
                    }
                }

            }
            override fun onErrorEvent(p0: Int, p1: Int) {
                Log.i("Prop Error", "$p0 , $p1")
            }

        },VENDOR_EXTENSION_STRING_DTC_PROPERTY, CarPropertyManager.SENSOR_RATE_FASTEST)

        carPropertyManager.registerCallback(object : CarPropertyManager.CarPropertyEventCallback{
            override fun onChangeEvent(p0: CarPropertyValue<*>?) {
                for (i in 1..8) {
                    UdsData.updateExist(i,0)
                }
                // show all list items (dtcs)
                recycler?.apply {
                    layoutManager = LinearLayoutManager(requireContext())
                    adapter = UdsAdapter(UdsData.items.filter { it.exist == 1 }) { itemId ->
                        findNavController().navigate(
                            R.id.action_uds_list_to_uds_detail,
                            Bundle().apply { putInt("itemId", itemId) }
                        )
                    }
                }
            }

            override fun onErrorEvent(p0: Int, p1: Int) {
                Log.i("Prop Error", "$p0 , $p1")
            }

        },VENDOR_EXTENSION_INIT_UDS_PROPERTY, CarPropertyManager.SENSOR_RATE_FASTEST)


        lifecycleScope.launch {
            while (true) {

                carPropertyManager.setIntProperty(VENDOR_EXTENSION_STRING_DTC_PROPERTY,0,1)
                kotlinx.coroutines.delay(200)
            }
        }

        clear_btn = view.findViewById(R.id.clear_button)
        send_sms_btn = view.findViewById(R.id.send_sms_button)
        send_report_btn = view.findViewById(R.id.send_report_button)

        send_sms_btn.setOnClickListener {
            val gatewayIp = wifiSender.getGatewayIpAddress()
            if (gatewayIp == null) {
                Toast.makeText(requireContext(), "Wi-Fi gateway not found", Toast.LENGTH_SHORT).show()
                return@setOnClickListener
            }

            val message = "01553524236:Service Center Message"
            wifiSender.sendMessage(gatewayIp, message) { success, error ->
                requireActivity().runOnUiThread {
                    if (success) {
                        Toast.makeText(requireContext(), "SMS sent via Wi-Fi", Toast.LENGTH_SHORT).show()
                    } else {
                        Toast.makeText(requireContext(), "Failed to send SMS: $error", Toast.LENGTH_LONG).show()
                    }
                }
            }
        }

        clear_btn.setOnClickListener {
            carPropertyManager.setIntProperty(VENDOR_EXTENSION_INIT_UDS_PROPERTY,0,1)
            // change status of all dtcs to be not exist
        }

        send_report_btn.setOnClickListener {
            val gatewayIp = wifiSender.getGatewayIpAddress()
            if (gatewayIp == null) {
                Toast.makeText(requireContext(), "Wi-Fi gateway not found", Toast.LENGTH_SHORT).show()
                return@setOnClickListener
            }

            val reportFile = wifiSender.createReportFile()
            if (reportFile == null) {
                Toast.makeText(requireContext(), "Failed to create report file", Toast.LENGTH_SHORT).show()
                return@setOnClickListener
            }

            wifiSender.sendFile(gatewayIp, reportFile) { success, error ->
                requireActivity().runOnUiThread {
                    if (success) {
                        Toast.makeText(requireContext(), "Report sent via Wi-Fi", Toast.LENGTH_SHORT).show()
                    } else {
                        Toast.makeText(requireContext(), "Failed to send report: $error", Toast.LENGTH_LONG).show()
                    }
                }
            }
        }
    }
}
