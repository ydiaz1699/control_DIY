#pragma once

#include <Arduino.h>
#include <functional>
#include <vector>

// ============================================================================
// BLE Driver - Bluetooth keyboard/media control
// Based on OMOTE's NimBLE implementation
// ============================================================================

/// BLE connection state
enum class BLEState : uint8_t {
    DISABLED,
    ADVERTISING,
    CONNECTED,
    PAIRED
};

/// BLE Driver (NimBLE-based keyboard/consumer control)
class BLEDriver {
public:
    static BLEDriver& instance();

    void init();
    void update();

    // Connection management
    void startAdvertising();
    void stopAdvertising();
    void disconnect();
    BLEState getState() const { return state_; }
    bool isConnected() const { return state_ == BLEState::CONNECTED || state_ == BLEState::PAIRED; }

    // Pairing
    void startPairing();
    bool isPairing() const { return pairing_; }
    int getBondedDeviceCount() const;
    void clearBonds();

    // Key sending
    void sendKey(uint8_t keyCode);
    void sendMediaKey(uint16_t mediaKey);
    void sendConsumerControl(uint16_t usage);

    // Common media keys
    void playPause();
    void volumeUp();
    void volumeDown();
    void mute();
    void nextTrack();
    void prevTrack();

    // Navigation keys (for smart TVs)
    void sendUp();
    void sendDown();
    void sendLeft();
    void sendRight();
    void sendSelect();
    void sendBack();
    void sendHome();

    // Callbacks
    using StateCallback = std::function<void(BLEState state)>;
    void onStateChange(StateCallback cb) { stateCb_ = cb; }

private:
    BLEDriver() = default;

    BLEState state_ = BLEState::DISABLED;
    bool pairing_ = false;
    StateCallback stateCb_ = nullptr;
};

// Global helper for CommandRouter
void ble_send_key(uint8_t key);
