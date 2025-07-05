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
import android.car.Car
import android.car.hardware.CarPropertyValue
import android.car.hardware.property.CarPropertyManager
import android.content.Intent

data class SpeedometerConfig(
    val minSpeed: Float = 0f,
    val maxSpeed: Float = 100f,
    val color: Int = Color.BLUE
)

class MainActivity : AppCompatActivity() {
    private lateinit var navController: NavController

    val VENDOR_EXTENSION_RPM_UDS_PROPERTY:Int = 0x21400104

    val VENDOR_EXTENSION_SPEED_UDS_PROPERTY:Int = 0x21400105

    val VENDOR_EXTENSION_OILTEMP_UDS_PROPERTY:Int = 0x21400106

    val VENDOR_EXTENSION_AIRFLOW_UDS_PROPERTY:Int = 0x21400107

    val VENDOR_EXTENSION_TIREPRES_UDS_PROPERTY:Int = 0x21400108

    val VENDOR_EXTENSION_STRING_DTC_PROPERTY:Int = 0x21100109

    val VENDOR_EXTENSION_INIT_UDS_PROPERTY:Int = 0x2140010A




    lateinit var carPropertyManager:CarPropertyManager

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
        //hideSystemBars()
        setContentView(R.layout.activity_main)

        val car =  Car.createCar(this.applicationContext)
	

        carPropertyManager = car!!.getCarManager(Car.PROPERTY_SERVICE) as CarPropertyManager

        carPropertyManager.registerCallback(object : CarPropertyManager.CarPropertyEventCallback{
            override fun onChangeEvent(p0: CarPropertyValue<*>?) {
                val value = p0?.value as? Int ?: 0
                updateSpeedometer(0, value.toFloat())            }

            override fun onErrorEvent(p0: Int, p1: Int) {
                Log.i("Prop Error", "$p0 , $p1")
            }

        },VENDOR_EXTENSION_RPM_UDS_PROPERTY, CarPropertyManager.SENSOR_RATE_FASTEST)

        carPropertyManager.registerCallback(object : CarPropertyManager.CarPropertyEventCallback{
            override fun onChangeEvent(p0: CarPropertyValue<*>?) {
                val value = p0?.value as? Int ?: 0
                updateSpeedometer(1, value.toFloat())            }

            override fun onErrorEvent(p0: Int, p1: Int) {
                Log.i("Prop Error", "$p0 , $p1")
            }

        },VENDOR_EXTENSION_SPEED_UDS_PROPERTY, CarPropertyManager.SENSOR_RATE_FASTEST)

        carPropertyManager.registerCallback(object : CarPropertyManager.CarPropertyEventCallback{
            override fun onChangeEvent(p0: CarPropertyValue<*>?) {
                updateDiagnostic("temperature", ((p0?.value ?: 0.0) as Int).toFloat())
            }

            override fun onErrorEvent(p0: Int, p1: Int) {
                Log.i("Prop Error", "$p0 , $p1")
            }

        },VENDOR_EXTENSION_OILTEMP_UDS_PROPERTY, CarPropertyManager.SENSOR_RATE_FASTEST)

        carPropertyManager.registerCallback(object : CarPropertyManager.CarPropertyEventCallback{
            override fun onChangeEvent(p0: CarPropertyValue<*>?) {
                updateDiagnostic("battery", ((p0?.value ?: 0.0) as Int).toFloat())
            }

            override fun onErrorEvent(p0: Int, p1: Int) {
                Log.i("Prop Error", "$p0 , $p1")
            }

        },VENDOR_EXTENSION_AIRFLOW_UDS_PROPERTY, CarPropertyManager.SENSOR_RATE_FASTEST)

        carPropertyManager.registerCallback(object : CarPropertyManager.CarPropertyEventCallback{
            override fun onChangeEvent(p0: CarPropertyValue<*>?) {
                updateDiagnostic("tirePressure", ((p0?.value ?: 0.0) as Int).toFloat())
            }

            override fun onErrorEvent(p0: Int, p1: Int) {
                Log.i("Prop Error", "$p0 , $p1")
            }

        },VENDOR_EXTENSION_TIREPRES_UDS_PROPERTY, CarPropertyManager.SENSOR_RATE_FASTEST)




//        lifecycleScope.launch {
//            while (true) {
//                updateSpeedometer(
//                    1,
//                    Random.nextFloat() * (config1.maxSpeed - config1.minSpeed) + config1.minSpeed
//                )
//                updateSpeedometer(
//                    2,
//                    Random.nextFloat() * (config2.maxSpeed - config2.minSpeed) + config2.minSpeed
//                )
//                updateSpeedometer(1, (50..100).random().toFloat())
//                updateSpeedometer(2, (50..100).random().toFloat())
//                updateDiagnostic("temperature", (180..220).random().toFloat())
//                updateDiagnostic("battery", (12..14).random().toFloat())
//                updateDiagnostic("tirePressure", (30..35).random().toFloat())
//                kotlinx.coroutines.delay(3500)
//            }
//        }

        lifecycleScope.launch {
            while (true) {

                carPropertyManager.setIntProperty(VENDOR_EXTENSION_RPM_UDS_PROPERTY,0,1)

                carPropertyManager.setIntProperty(VENDOR_EXTENSION_SPEED_UDS_PROPERTY,0,1)

                carPropertyManager.setIntProperty(VENDOR_EXTENSION_OILTEMP_UDS_PROPERTY,0,1)

                carPropertyManager.setIntProperty(VENDOR_EXTENSION_AIRFLOW_UDS_PROPERTY,0,1)

                carPropertyManager.setIntProperty(VENDOR_EXTENSION_TIREPRES_UDS_PROPERTY,0,1)

                kotlinx.coroutines.delay(200)
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
