/**
 * ble_keyboard.cpp — BLE HID Keyboard Implementation
 * 
 * Uses NimBLE for efficient BLE HID keyboard emulation
 * Based on OMOTE keyboard_ble_hal_esp32.cpp
 */

#include "ble_keyboard.h"
#include "../config/user_config.h"

#if ENABLE_BLE_KEYBOARD

#include <NimBLEDevice.h>

// BLE HID Report Map (keyboard)
static const uint8_t HID_REPORT_MAP[] = {
    0x05, 0x01,  // Usage Page (Generic Desktop)
    0x09, 0x06,  // Usage (Keyboard)
    0xA1, 0x01,  // Collection (Application)
    0x85, 0x01,  //   Report ID (1)
    0x05, 0x07,  //   Usage Page (Key Codes)
    0x19, 0xE0,  //   Usage Min (224)
    0x29, 0xE7,  //   Usage Max (231)
    0x15, 0x00,  //   Logical Min (0)
    0x25, 0x01,  //   Logical Max (1)
    0x75, 0x01,  //   Report Size (1)
    0x95, 0x08,  //   Report Count (8)
    0x81, 0x02,  //   Input (Variable)
    0x95, 0x01,  //   Report Count (1)
    0x75, 0x08,  //   Report Size (8)
    0x81, 0x01,  //   Input (Constant)
    0x95, 0x05,  //   Report Count (5)
    0x75, 0x01,  //   Report Size (1)
    0x05, 0x08,  //   Usage Page (LEDs)
    0x19, 0x01,  //   Usage Min (1)
    0x29, 0x05,  //   Usage Max (5)
    0x91, 0x02,  //   Output (Variable)
    0x95, 0x01,  //   Report Count (1)
    0x75, 0x03,  //   Report Size (3)
    0x91, 0x01,  //   Output (Constant)
    0x95, 0x06,  //   Report Count (6)
    0x75, 0x08,  //   Report Size (8)
    0x15, 0x00,  //   Logical Min (0)
    0x25, 0x65,  //   Logical Max (101)
    0x05, 0x07,  //   Usage Page (Key Codes)
    0x19, 0x00,  //   Usage Min (0)
    0x29, 0x65,  //   Usage Max (101)
    0x81, 0x00,  //   Input (Data, Array)
    0xC0,        // End Collection
    // Consumer Control
    0x05, 0x0C,  // Usage Page (Consumer)
    0x09, 0x01,  // Usage (Consumer Control)
    0xA1, 0x01,  // Collection (Application)
    0x85, 0x02,  //   Report ID (2)
    0x15, 0x00,  //   Logical Min (0)
    0x26, 0xFF, 0x03,  // Logical Max (1023)
    0x19, 0x00,  //   Usage Min (0)
    0x2A, 0xFF, 0x03,  // Usage Max (1023)
    0x75, 0x10,  //   Report Size (16)
    0x95, 0x01,  //   Report Count (1)
    0x81, 0x00,  //   Input (Data, Array)
    0xC0,        // End Collection
};

static NimBLEServer *pServer = nullptr;
static NimBLECharacteristic *pInputChar = nullptr;
static NimBLECharacteristic *pConsumerChar = nullptr;
static bool deviceConnected = false;

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) override {
        deviceConnected = true;
        Serial.println("[BLE] Client connected");
    }
    void onDisconnect(NimBLEServer* pServer) override {
        deviceConnected = false;
        Serial.println("[BLE] Client disconnected");
        NimBLEDevice::startAdvertising();
    }
};

void ble_keyboard_init() {
    NimBLEDevice::init(BLE_DEVICE_NAME);
    NimBLEDevice::setPower(ESP_PWR_LVL_P3);
    NimBLEDevice::setSecurityAuth(true, true, true);
    
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());
    
    // HID Service
    NimBLEService *pHidService = pServer->createService("1812");
    
    // HID Report Map
    NimBLECharacteristic *pReportMap = pHidService->createCharacteristic(
        "2A4B", NIMBLE_PROPERTY::READ);
    pReportMap->setValue((uint8_t*)HID_REPORT_MAP, sizeof(HID_REPORT_MAP));
    
    // HID Information
    uint8_t hidInfo[] = {0x01, 0x11, 0x00, 0x02};
    NimBLECharacteristic *pHidInfo = pHidService->createCharacteristic(
        "2A4A", NIMBLE_PROPERTY::READ);
    pHidInfo->setValue(hidInfo, 4);
    
    // Input Report (keyboard)
    pInputChar = pHidService->createCharacteristic(
        "2A4D", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    
    // Consumer Report
    pConsumerChar = pHidService->createCharacteristic(
        "2A4D", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    
    pHidService->start();
    
    // Battery Service
    NimBLEService *pBattService = pServer->createService("180F");
    NimBLECharacteristic *pBattLevel = pBattService->createCharacteristic(
        "2A19", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    uint8_t battLevel = 100;
    pBattLevel->setValue(&battLevel, 1);
    pBattService->start();
    
    // Device Info Service
    NimBLEService *pDevInfoService = pServer->createService("180A");
    NimBLECharacteristic *pManufacturer = pDevInfoService->createCharacteristic(
        "2A29", NIMBLE_PROPERTY::READ);
    pManufacturer->setValue(BLE_MANUFACTURER);
    pDevInfoService->start();
    
    // Start advertising
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID("1812");
    pAdvertising->setAppearance(961); // Keyboard
    pAdvertising->start();
    
    Serial.printf("[BLE] Keyboard \"%s\" initialized and advertising\n", BLE_DEVICE_NAME);
}

void ble_keyboard_loop() {
    // NimBLE handles everything in callbacks
}

bool ble_keyboard_is_connected() {
    return deviceConnected;
}

void ble_keyboard_write(uint8_t key) {
    if (!deviceConnected) return;
    
    uint8_t report[8] = {0};
    report[2] = key;
    pInputChar->setValue(report, 8);
    pInputChar->notify();
    
    delay(20);
    
    memset(report, 0, 8);
    pInputChar->setValue(report, 8);
    pInputChar->notify();
}

void ble_keyboard_press(uint8_t key) {
    if (!deviceConnected) return;
    uint8_t report[8] = {0};
    report[2] = key;
    pInputChar->setValue(report, 8);
    pInputChar->notify();
}

void ble_keyboard_release(uint8_t key) {
    if (!deviceConnected) return;
    uint8_t report[8] = {0};
    pInputChar->setValue(report, 8);
    pInputChar->notify();
}

void ble_keyboard_release_all() {
    if (!deviceConnected) return;
    uint8_t report[8] = {0};
    pInputChar->setValue(report, 8);
    pInputChar->notify();
}

void ble_keyboard_send_string(const char* str) {
    // Simplified: only ASCII
    for (int i = 0; str[i]; i++) {
        uint8_t key = 0;
        if (str[i] >= 'a' && str[i] <= 'z') key = str[i] - 'a' + 4;
        else if (str[i] >= 'A' && str[i] <= 'Z') key = str[i] - 'A' + 4;
        else if (str[i] == ' ') key = 0x2C;
        if (key) ble_keyboard_write(key);
    }
}

// Consumer control keys
static void sendConsumer(uint16_t usage) {
    if (!deviceConnected || !pConsumerChar) return;
    uint8_t report[2] = {(uint8_t)(usage & 0xFF), (uint8_t)((usage >> 8) & 0xFF)};
    pConsumerChar->setValue(report, 2);
    pConsumerChar->notify();
    delay(20);
    memset(report, 0, 2);
    pConsumerChar->setValue(report, 2);
    pConsumerChar->notify();
}

void ble_keyboard_media_play_pause() { sendConsumer(0xCD); }
void ble_keyboard_media_next()       { sendConsumer(0xB5); }
void ble_keyboard_media_prev()       { sendConsumer(0xB6); }
void ble_keyboard_media_vol_up()     { sendConsumer(0xE9); }
void ble_keyboard_media_vol_down()   { sendConsumer(0xEA); }
void ble_keyboard_media_mute()       { sendConsumer(0xE2); }

void ble_keyboard_shutdown() {
    NimBLEDevice::deinit(true);
    deviceConnected = false;
}

#else
// Stubs when BLE disabled
void ble_keyboard_init() {}
void ble_keyboard_loop() {}
bool ble_keyboard_is_connected() { return false; }
void ble_keyboard_write(uint8_t key) {}
void ble_keyboard_press(uint8_t key) {}
void ble_keyboard_release(uint8_t key) {}
void ble_keyboard_release_all() {}
void ble_keyboard_send_string(const char* str) {}
void ble_keyboard_media_play_pause() {}
void ble_keyboard_media_next() {}
void ble_keyboard_media_prev() {}
void ble_keyboard_media_vol_up() {}
void ble_keyboard_media_vol_down() {}
void ble_keyboard_media_mute() {}
void ble_keyboard_shutdown() {}
#endif
