package com.example.diagnostic_android_app

import android.graphics.Color
import android.os.Bundle
import android.view.View
import android.widget.TextView
import androidx.core.content.ContextCompat
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
                indicator.width = 15f
                speedTo(MainActivity.speed1Flow.value, 0)
                withTremble = false
            }
        }
        speedometer2?.apply {
            post {
                minSpeed = MainActivity.config2.minSpeed
                maxSpeed = MainActivity.config2.maxSpeed
                trianglesColor = Color.parseColor("#333333")
                indicator.width = 15f
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
                        val temp = value.coerceIn(-40f, 215f)
                        temperatureText?.text = String.format("%.1f°C", temp)

                        val color = when {
                            temp > 115f -> R.color.critical_red
                            temp > 105f -> R.color.warning_orange
                            temp < 70f -> R.color.warning_blue
                            else -> R.color.normal_green
                        }
                        temperatureText?.setTextColor(ContextCompat.getColor(requireContext(), color))
                    }
                }

                launch {
                    MainActivity.Air_flow_Flow.collect { value ->
                        val airFlow = value.coerceIn(0f, 655f)
                        batteryText?.text = String.format("%.1f", airFlow)

                        val color = when {
                            airFlow > 450f || airFlow < 50f -> R.color.warning_orange
                            else -> R.color.normal_green
                        }
                        batteryText?.setTextColor(ContextCompat.getColor(requireContext(), color))
                    }
                }

                launch {
                    MainActivity.tirePressureFlow.collect { value ->
                        val pressure = value.coerceIn(0f, 42f)
                        tirePressureText?.text = String.format("%.1f PSI", pressure)

                        val color = when {
                            pressure < 28f || pressure > 36f -> R.color.critical_red
                            pressure < 30f || pressure > 35f -> R.color.warning_orange
                            else -> R.color.normal_green
                        }
                        tirePressureText?.setTextColor(ContextCompat.getColor(requireContext(), color))
                    }
                }


            }
        }
    }
}
