package com.example.diagnostic_android_app

import androidx.annotation.DrawableRes

data class UdsItem(
    val id: Int,
    val name: String,
    val details: String,
    @DrawableRes val iconRes: Int,
    val exist : Int,
    val needsGarageSupport: Boolean
)


object UdsData {
    private val _items = mutableListOf(
        UdsItem(1, "Speed Sensor", "Monitors speed", R.drawable.carspeed1, 1, false),
        UdsItem(2, "Oil Temp Sensor", "Tracks oil temp", R.drawable.thermometer1, 1, true),
        UdsItem(3, "MAF Sensor", "Measures airflow", R.drawable.airflow1, 0, true)
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

