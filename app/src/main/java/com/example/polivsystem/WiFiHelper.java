package com.example.polivsystem;

import android.content.Context;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiInfo;
import android.util.Log;

public class WiFiHelper {
    private static final String TAG = "WiFiHelper";
    private Context context;

    public WiFiHelper(Context context) {
        this.context = context;
    }

    // Проверка подключения wifi  к ESP32
    public boolean isConnectedToESP32() {
        try {
            WifiManager wifiManager = (WifiManager)
                    context.getApplicationContext().getSystemService(Context.WIFI_SERVICE);

            if (wifiManager == null) {
                Log.e(TAG, "WifiManager is null");
                return false;
            }

            WifiInfo wifiInfo = wifiManager.getConnectionInfo();
            String ssid = wifiInfo.getSSID();

            Log.d(TAG, "Current SSID: " + ssid);

            // Убираем кавычки если есть
            if (ssid != null && ssid.startsWith("\"") && ssid.endsWith("\"")) {
                ssid = ssid.substring(1, ssid.length() - 1);
            }

            boolean isConnected = ssid != null && ssid.equals("PolivSystem");
            Log.d(TAG, "Connected to PolivSystem: " + isConnected);
            return isConnected;

        } catch (Exception e) {
            Log.e(TAG, "Error checking WiFi: " + e.getMessage());
            return false;
        }
    }
}