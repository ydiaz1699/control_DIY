#include "ble_driver.h"
#include "../config.h"

#if ENABLE_BLE
#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>

// HID Report Descriptor for Keyboard + Consumer Control
static const uint8_t hidReportDescriptor[] = {
    // Keyboard
    0x05, 0x01, // Usage Page (Generic Desktop)
    0x09, 0x06, // Usage (Keyboard)
    0xA1, 0x01, // Collection (Application)
    0x85, 0x01, //   Report ID (1)
    0x05, 0x07, //   Usage Page (Keyboard/Keypad)
    0x19, 0xE0, //   Usage Minimum (Left Control)
    0x29, 0xE7, //   Usage Maximum (Right GUI)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x08, //   Report Count (8)
    0x81, 0x02, //   Input (Data, Variable, Absolute)
    0x95, 0x01, //   Report Count (1)
    0x75, 0x08, //   Report Size (8)
    0x81, 0x01, //   Input (Constant)
    0x95, 0x06, //   Report Count (6)
    0x75, 0x08, //   Report Size (8)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x65, //   Logical Maximum (101)
    0x05, 0x07, //   Usage Page (Keyboard/Keypad)
    0x19, 0x00, //   Usage Minimum (0)
    0x29, 0x65, //   Usage Maximum (101)
    0x81, 0x00, //   Input (Data, Array)
    0xC0,       // End Collection

    // Consumer Control
    0x05, 0x0C, // Usage Page (Consumer)
    0x09, 0x01, // Usage (Consumer Control)
    0xA1, 0x01, // Collection (Application)
    0x85, 0x02, //   Report ID (2)
    0x15, 0x00, //   Logical Minimum (0)
    0x26, 0xFF, 0x03, // Logical Maximum (1023)
    0x19, 0x00, //   Usage Minimum (0)
    0x2A, 0xFF, 0x03, // Usage Maximum (1023)
    0x75, 0x10, //   Report Size (16)
    0x95, 0x01, //   Report Count (1)
    0x81, 0x00, //   Input (Data, Array)
    0xC0        // End Collection
};

static NimBLEHIDDevice* hidDevice = nullptr;
static NimBLECharacteristic* inputKeyboard = nullptr;
static NimBLECharacteristic* inputConsumer = nullptr;
#endif

BLEDriver& BLEDriver::instance() {
    static BLEDriver inst;
    return inst;
}

void BLEDriver::init() {
#if ENABLE_BLE
    NimBLEDevice::init("ControlDIY-Mando");
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

    NimBLEServer* server = NimBLEDevice::createServer();
    hidDevice = new NimBLEHIDDevice(server);

    inputKeyboard = hidDevice->inputReport(1);
    inputConsumer = hidDevice->inputReport(2);

    hidDevice->reportMap((uint8_t*)hidReportDescriptor, sizeof(hidReportDescriptor));
    hidDevice->pnp(0x02, 0x05AC, 0x820A, 0x0210);  // Apple-like PnP
    hidDevice->hidInfo(0x00, 0x01);

    hidDevice->startServices();

    state_ = BLEState::ADVERTISING;
    startAdvertising();
#else
    state_ = BLEState::DISABLED;
#endif
}

void BLEDriver::update() {
#if ENABLE_BLE
    // State management handled by NimBLE callbacks
#endif
}

void BLEDriver::startAdvertising() {
#if ENABLE_BLE
    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
    advertising->setAppearance(HID_KEYBOARD);
    advertising->addServiceUUID(hidDevice->hidService()->getUUID());
    advertising->start();
    state_ = BLEState::ADVERTISING;
#endif
}

void BLEDriver::stopAdvertising() {
#if ENABLE_BLE
    NimBLEDevice::getAdvertising()->stop();
#endif
}

void BLEDriver::disconnect() {
#if ENABLE_BLE
    // Disconnect all peers
    NimBLEDevice::getServer()->disconnect(0);
    state_ = BLEState::ADVERTISING;
#endif
}

void BLEDriver::startPairing() {
    pairing_ = true;
    startAdvertising();
}

int BLEDriver::getBondedDeviceCount() const {
#if ENABLE_BLE
    return NimBLEDevice::getNumBonds();
#else
    return 0;
#endif
}

void BLEDriver::clearBonds() {
#if ENABLE_BLE
    NimBLEDevice::deleteAllBonds();
#endif
}

void BLEDriver::sendKey(uint8_t keyCode) {
#if ENABLE_BLE
    if (!isConnected()) return;
    uint8_t report[8] = {0};
    report[2] = keyCode;
    inputKeyboard->setValue(report, sizeof(report));
    inputKeyboard->notify();
    delay(10);
    // Release
    memset(report, 0, sizeof(report));
    inputKeyboard->setValue(report, sizeof(report));
    inputKeyboard->notify();
#endif
}

void BLEDriver::sendMediaKey(uint16_t mediaKey) {
    sendConsumerControl(mediaKey);
}

void BLEDriver::sendConsumerControl(uint16_t usage) {
#if ENABLE_BLE
    if (!isConnected()) return;
    uint8_t report[2] = {(uint8_t)(usage & 0xFF), (uint8_t)((usage >> 8) & 0xFF)};
    inputConsumer->setValue(report, sizeof(report));
    inputConsumer->notify();
    delay(10);
    // Release
    report[0] = 0; report[1] = 0;
    inputConsumer->setValue(report, sizeof(report));
    inputConsumer->notify();
#endif
}

void BLEDriver::playPause()  { sendConsumerControl(0xCD); }
void BLEDriver::volumeUp()   { sendConsumerControl(0xE9); }
void BLEDriver::volumeDown() { sendConsumerControl(0xEA); }
void BLEDriver::mute()       { sendConsumerControl(0xE2); }
void BLEDriver::nextTrack()  { sendConsumerControl(0xB5); }
void BLEDriver::prevTrack()  { sendConsumerControl(0xB6); }

void BLEDriver::sendUp()     { sendKey(0x52); }
void BLEDriver::sendDown()   { sendKey(0x51); }
void BLEDriver::sendLeft()   { sendKey(0x50); }
void BLEDriver::sendRight()  { sendKey(0x4F); }
void BLEDriver::sendSelect() { sendKey(0x28); }  // Enter
void BLEDriver::sendBack()   { sendKey(0x29); }  // Escape
void BLEDriver::sendHome()   { sendKey(0x4A); }  // Home

// Global helper
void ble_send_key(uint8_t key) {
    BLEDriver::instance().sendKey(key);
}
