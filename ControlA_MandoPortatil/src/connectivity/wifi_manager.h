#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <functional>

// ============================================================================
// WiFi Manager - Connection management with fallback AP mode
// ============================================================================

class WiFiManager {
public:
    static WiFiManager& instance();

    void init();
    void update();

    // Connection
    bool connect(const String& ssid, const String& password);
    void disconnect();
    bool isConnected() const;
    String getIP() const;
    int getRSSI() const;

    // Fallback AP mode
    void startAP(const String& apName = "ControlDIY-Setup");
    void stopAP();
    bool isAPMode() const { return apMode_; }

    // Power optimization
    void setPowerSaveMode(bool enabled);
    void setTxPower(int dBm);  // 2-20 dBm

    // Callbacks
    using ConnCallback = std::function<void(bool connected)>;
    void onConnectionChange(ConnCallback cb) { connCb_ = cb; }

private:
    WiFiManager() = default;

    bool apMode_ = false;
    bool connecting_ = false;
    unsigned long lastReconnectMs_ = 0;
    int reconnectAttempts_ = 0;

    ConnCallback connCb_ = nullptr;
};
