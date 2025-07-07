package com.example.diagnostic_android_app

import android.graphics.Color
import android.os.Bundle
import android.view.View
import android.widget.TextView
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import androidx.lifecycle.repeatOnLifecycle
import com.github.anastr.speedviewlib.AwesomeSpeedometer
import kotlinx.coroutines.launch

class DashboardFragment : Fragment(R.layout.fragment_dashboard) {

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        val speedometer1 = view.findViewById<AwesomeSpeedometer>(R.id.speedometer1)
        val speedometer2 = view.findViewById<AwesomeSpeedometer>(R.id.speedometer2)
        val temperatureText = view.findViewById<TextView>(R.id.temperature_txt)
        val batteryText = view.findViewById<TextView>(R.id.BatteryLevel_txt)
        val tirePressureText = view.findViewById<TextView>(R.id.TirePressure_txt)
        speedometer1?.apply {
            post {
                minSpeed = MainActivity.config1.minSpeed
                maxSpeed = MainActivity.config1.maxSpeed
//                setSpeedometerColor(MainActivity.config1.color)
                indicator.width = 15f
//                indicator.color = Color.WHITE
                speedTo(MainActivity.speed1Flow.value, 0)
                withTremble = false
            }
        }
        speedometer2?.apply {
            post {
                minSpeed = MainActivity.config2.minSpeed
                maxSpeed = MainActivity.config2.maxSpeed
//                setSpeedometerColor(MainActivity.config2.color)
                trianglesColor = Color.parseColor("#333333")
                indicator.width = 15f
//                indicator.color = Color.WHITE
                speedTo(MainActivity.speed2Flow.value, 0)
                withTremble = false
            }
        }

        viewLifecycleOwner.lifecycleScope.launch {
            viewLifecycleOwner.repeatOnLifecycle(androidx.lifecycle.Lifecycle.State.STARTED) {
                launch {
                    MainActivity.speed1Flow.collect { newSpeed ->
                        speedometer1?.speedTo(newSpeed)
                    }
                }
                launch {
                    MainActivity.speed2Flow.collect { newSpeed ->
                        speedometer2?.speedTo(newSpeed)
                    }
                }
                launch {
                    MainActivity.temperatureFlow.collect { value ->
                        temperatureText?.text = String.format("%.1f°F", value)
                    }
                }
                launch {
                    MainActivity.batteryFlow.collect { value ->
                        batteryText?.text = String.format("%.1fV", value)
                    }
                }
                launch {
                    MainActivity.tirePressureFlow.collect { value ->
                        tirePressureText?.text = String.format("%.1f PSI", value)
                    }
                }

            }
        }
    }
}
