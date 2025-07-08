package com.example.diagnostic_android_app

import android.os.Bundle
import android.view.View
import android.widget.Button
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.navigation.fragment.findNavController
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView

class UdsListFragment : Fragment(R.layout.fragment_uds_list) {

    private lateinit var clear_btn: Button
    private lateinit var send_sms_btn: Button
    private lateinit var send_report_btn: Button

    private lateinit var wifiSender: WifiSender

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        wifiSender = WifiSender(requireContext())

        val recycler = view.findViewById<RecyclerView>(R.id.uds_list)
        recycler?.apply {
            layoutManager = LinearLayoutManager(requireContext())
            adapter = UdsAdapter(UdsData.items.filter { it.exist == 1 }) { itemId ->
                findNavController().navigate(
                    R.id.action_uds_list_to_uds_detail,
                    Bundle().apply { putInt("itemId", itemId) }
                )
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
