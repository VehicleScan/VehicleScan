package com.example.diagnostic_android_app

import android.graphics.Color
import android.os.Bundle
import android.view.View
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

        speedometer1?.apply {
            post {
                minSpeed = MainActivity.config1.minSpeed
                maxSpeed = MainActivity.config1.maxSpeed
                setSpeedometerColor(MainActivity.config1.color)
                trianglesColor = Color.parseColor("#333333")
                indicator.width = 15f
                indicator.color = Color.WHITE
                speedTo(MainActivity.speed1Flow.value, 0)
                withTremble = false
            }
        }
        speedometer2?.apply {
            post {
                minSpeed = MainActivity.config2.minSpeed
                maxSpeed = MainActivity.config2.maxSpeed
                setSpeedometerColor(MainActivity.config2.color)
                trianglesColor = Color.parseColor("#333333")
                indicator.width = 15f
                indicator.color = Color.WHITE
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

            }
        }
    }
}
