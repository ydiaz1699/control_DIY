#include "wifi_manager.h"
#include "../config.h"

WiFiManager& WiFiManager::instance() {
    static WiFiManager inst;
    return inst;
}

void WiFiManager::init() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    // Power save mode for battery life
    setPowerSaveMode(true);

    // Try to connect with saved credentials
    if (strlen(WiFiConfig::DEFAULT_SSID) > 0) {
        connect(WiFiConfig::DEFAULT_SSID, WiFiConfig::DEFAULT_PASSWORD);
    }
}

void WiFiManager::update() {
    if (apMode_) return;

    if (!isConnected() && !connecting_) {
        unsigned long now = millis();
        if (now - lastReconnectMs_ > (unsigned long)WiFiConfig::RECONNECT_INTERVAL_MS) {
            lastReconnectMs_ = now;
            reconnectAttempts_++;

            if (reconnectAttempts_ > 5) {
                // Start AP mode as fallback
                startAP();
            } else {
                WiFi.reconnect();
            }
        }
    }
}

bool WiFiManager::connect(const String& ssid, const String& password) {
    connecting_ = true;
    reconnectAttempts_ = 0;

    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long start = millis();
    while (!isConnected() && (millis() - start) < (unsigned long)WiFiConfig::CONNECT_TIMEOUT_MS) {
        delay(100);
    }

    connecting_ = false;
    bool connected = isConnected();

    if (connCb_) {
        connCb_(connected);
    }

    return connected;
}

void WiFiManager::disconnect() {
    WiFi.disconnect(true);
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getIP() const {
    return WiFi.localIP().toString();
}

int WiFiManager::getRSSI() const {
    return WiFi.RSSI();
}

void WiFiManager::startAP(const String& apName) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(apName.c_str());
    apMode_ = true;
}

void WiFiManager::stopAP() {
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    apMode_ = false;
}

void WiFiManager::setPowerSaveMode(bool enabled) {
    if (enabled) {
        WiFi.setSleep(WIFI_PS_MIN_MODEM);
    } else {
        WiFi.setSleep(WIFI_PS_NONE);
    }
}

void WiFiManager::setTxPower(int dBm) {
    wifi_power_t power = (wifi_power_t)(dBm * 4);  // ESP-IDF uses quarter dBm
    WiFi.setTxPower(power);
}
