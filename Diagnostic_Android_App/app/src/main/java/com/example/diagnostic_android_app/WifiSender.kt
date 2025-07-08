package com.example.diagnostic_android_app

import android.content.Context
import android.net.wifi.WifiManager
import android.util.Log
import java.io.*
import java.net.ConnectException
import java.net.Socket

class WifiSender(private val context: Context) {

    companion object {
        private const val TAG = "WifiSender"
        private const val SERVER_PORT = 8888
    }

    /**
     * Returns the gateway IP address of the connected Wi-Fi network.
     */
    fun getGatewayIpAddress(): String? {
        val wifiManager = context.getSystemService(Context.WIFI_SERVICE) as? WifiManager
        if (wifiManager == null || !wifiManager.isWifiEnabled) return null

        val gateway = wifiManager.dhcpInfo.gateway
        return if (gateway == 0) null else String.format(
            "%d.%d.%d.%d",
            gateway and 0xff,
            gateway shr 8 and 0xff,
            gateway shr 16 and 0xff,
            gateway shr 24 and 0xff
        )
    }

    /**
     * Sends a message over Wi-Fi to the gateway or given IP.
     */
    fun sendMessage(deviceAddress: String, message: String, onResult: (success: Boolean, errorMsg: String?) -> Unit) {
        Thread {
            try {
                Socket(deviceAddress, SERVER_PORT).use { socket ->
                    PrintWriter(socket.getOutputStream(), true).use { writer ->
                        writer.println(message)
                    }
                }
                onResult(true, null)
            } catch (e: ConnectException) {
                Log.e(TAG, "Connection failed: ${e.message}")
                onResult(false, "Connection failed: ${e.message}")
            } catch (e: IOException) {
                Log.e(TAG, "Send failed: ${e.message}")
                onResult(false, "Send failed: ${e.message}")
            }
        }.start()
    }

    /**
     * Sends a report file over Wi-Fi to the given IP address.
     */
    fun sendFile(deviceAddress: String, file: File, onResult: (success: Boolean, errorMsg: String?) -> Unit) {
        Thread {
            try {
                Socket(deviceAddress, SERVER_PORT).use { socket ->
                    val outputStream = socket.getOutputStream()
                    val headerWriter = PrintWriter(outputStream, true)
                    headerWriter.println("FILE_TRANSFER:")
                    headerWriter.flush()

                    FileInputStream(file).use { fis ->
                        val buffer = ByteArray(1024)
                        var len: Int
                        while (fis.read(buffer).also { len = it } != -1) {
                            outputStream.write(buffer, 0, len)
                        }
                        outputStream.flush()
                    }
                }
                onResult(true, null)
            } catch (e: IOException) {
                Log.e(TAG, "File send failed: ${e.message}")
                onResult(false, "File send failed: ${e.message}")
            }
        }.start()
    }

    /**
     * Creates report file (DTC codes) in internal storage.
     */
    fun createReportFile(): File? {
        val file = File(context.filesDir, "dtc_report.txt")
        return try {
            FileWriter(file).use { writer ->
                writer.write("P0100,P0200,P0107,P0207,P1234,P0102,U1600,B2245")
            }
            file
        } catch (e: IOException) {
            Log.e(TAG, "Error creating report file: ${e.message}")
            null
        }
    }
}
