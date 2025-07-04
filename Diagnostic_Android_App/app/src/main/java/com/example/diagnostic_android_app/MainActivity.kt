package com.example.diagnostic_android_app

import android.content.pm.ActivityInfo
import android.graphics.Color
import android.os.Bundle
import android.util.Log
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import androidx.navigation.NavController
import androidx.navigation.fragment.NavHostFragment
import androidx.navigation.ui.setupWithNavController
import com.google.android.material.bottomnavigation.BottomNavigationView
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch
import kotlin.random.Random

data class SpeedometerConfig(
    val minSpeed: Float = 0f,
    val maxSpeed: Float = 100f,
    val color: Int = Color.BLUE
)

class MainActivity : AppCompatActivity() {
    private lateinit var navController: NavController

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
        //hideSystemBars()
        setContentView(R.layout.activity_main)

        lifecycleScope.launch {
            while (true) {
                updateSpeedometer(
                    1,
                    Random.nextFloat() * (config1.maxSpeed - config1.minSpeed) + config1.minSpeed
                )
                updateSpeedometer(
                    2,
                    Random.nextFloat() * (config2.maxSpeed - config2.minSpeed) + config2.minSpeed
                )
                updateSpeedometer(1, (50..100).random().toFloat())
                updateSpeedometer(2, (50..100).random().toFloat())
                updateDiagnostic("temperature", (180..220).random().toFloat())
                updateDiagnostic("battery", (12..14).random().toFloat())
                updateDiagnostic("tirePressure", (30..35).random().toFloat())
                kotlinx.coroutines.delay(3500)
            }
        }

        val navHostFragment =
            supportFragmentManager.findFragmentById(R.id.nav_host_fragment) as? NavHostFragment
        if (navHostFragment == null) {
            Log.e("MainActivity", "NavHostFragment not found")
            return
        }
        navController = navHostFragment.navController

        findViewById<BottomNavigationView>(R.id.bottom_navigation)
            ?.setupWithNavController(navController)
    }

    private fun hideSystemBars() {
        if (packageManager.hasSystemFeature("android.hardware.type.automotive")) {
            window.decorView.systemUiVisibility =
                (View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        or View.SYSTEM_UI_FLAG_FULLSCREEN
                        or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY)
        }
    }


    fun updateDiagnostic(type: String, value: Float) {
        lifecycleScope.launch {
            when (type) {
                "temperature" -> _temperatureFlow.value = value
                "battery" -> _batteryFlow.value = value
                "tirePressure" -> _tirePressureFlow.value = value
            }
        }
    }

    fun updateSpeedometer(index: Int, newSpeed: Float) {
        when (index) {
            1 -> _speed1Flow.value = newSpeed
            2 -> _speed2Flow.value = newSpeed
        }
    }


    companion object {
        private val _speed1Flow = MutableStateFlow(0f)
        private val _speed2Flow = MutableStateFlow(0f)
        private val _temperatureFlow = MutableStateFlow(0f)
        private val _batteryFlow = MutableStateFlow(0f)
        private val _tirePressureFlow = MutableStateFlow(0f)
        val speed1Flow: StateFlow<Float> get() = _speed1Flow
        val speed2Flow: StateFlow<Float> get() = _speed2Flow
        val temperatureFlow: StateFlow<Float> get() = _temperatureFlow
        val batteryFlow: StateFlow<Float> get() = _batteryFlow
        val tirePressureFlow: StateFlow<Float> get() = _tirePressureFlow
        val config1 = SpeedometerConfig()
        val config2 = SpeedometerConfig()
    }
}
