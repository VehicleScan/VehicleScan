package com.example.diagnostic_android_app

import android.os.Bundle
import android.view.View
import android.widget.Button
import androidx.fragment.app.Fragment
import androidx.navigation.fragment.findNavController
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView

class UdsListFragment : Fragment(R.layout.fragment_uds_list) {

    lateinit var clear_btn : Button
    lateinit var send_sms_btn: Button
    lateinit var  send_report_btn: Button
    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

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
        clear_btn = view.findViewById<Button>(R.id.clear_button)
        send_sms_btn = view.findViewById<Button>(R.id.send_sms_button)
        send_report_btn = view.findViewById<Button>(R.id.send_report_button)

    }
}