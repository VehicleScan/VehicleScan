package com.example.diagnostic_android_app

import androidx.annotation.DrawableRes

data class UdsItem(
    val id: Int,
    val name: String,
    val details: String,
    @DrawableRes val iconRes: Int,
    val exist: Int,
    val needsGarageSupport: Boolean,
    val code: String? = null // optional new field
)



object UdsData {
    private val _items = mutableListOf(
        // === DTC Entries ===
        UdsItem(1, "Air Flow Sensor Issue", "The sensor that measures air going into the engine may not be working properly. This can affect fuel usage and performance.", R.drawable.air_flow, 0, false),
        UdsItem(2, "Fuel Injector Problem", "A problem was found in the fuel injector system, which can cause rough running or poor fuel efficiency.", R.drawable.fuel_injector, 0, false),
        UdsItem(3, "Air Pressure Sensor Issue", "The sensor that reads outside air pressure may be sending low signals. This might affect how the engine runs.", R.drawable.barometric_pressure, 0, false),
        UdsItem(4, "Throttle Response Timeout", "The engine's throttle did not respond in time. This might cause poor acceleration or delayed engine reaction.", R.drawable.throttle, 0, false),
        UdsItem(5, "Ignition Security Lockout", "The car's security system has stopped the engine from starting. This could be due to unauthorized access or a system fault.", R.drawable.ignition, 0, true),
        UdsItem(6, "Low Air Flow Signal", "Low air flow into the engine detected. This may cause poor performance or stalling.", R.drawable.air_flow, 0, true),
        UdsItem(7, "Immobilizer System Error", "The car's anti-theft system isn't communicating properly. This might stop the engine from starting.", R.drawable.immoblizer, 0, true),
        UdsItem(8, "ABS System Fault", "The anti-lock brake system has reported a problem. Braking may still work, but without ABS assistance.", R.drawable.abs, 0, true)
    )

    val items: List<UdsItem> = _items

    fun updateExist(itemId: Int, exist: Int) {
        synchronized(_items) {
            val item = _items.find { it.id == itemId }
            item?.let {
                val index = _items.indexOf(it)
                _items[index] = it.copy(exist = exist)
            }
        }
    }

    fun updateGarageSupport(itemId: Int, needsGarageSupport: Boolean) {
        synchronized(_items) {
            val item = _items.find { it.id == itemId }
            item?.let {
                val index = _items.indexOf(it)
                _items[index] = it.copy(needsGarageSupport = needsGarageSupport)
            }
        }
    }
}

