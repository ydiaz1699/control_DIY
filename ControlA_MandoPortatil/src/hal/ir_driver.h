#pragma once

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <functional>

// ============================================================================
// IR Driver - Transmit and receive infrared signals
// Based on OMOTE's IR implementation + Everything Remote's IR ideas
// ============================================================================

/// IR Protocol IDs (subset, expandable)
enum class IRProtocol : uint8_t {
    NEC = 1,
    SAMSUNG,
    SONY,
    RC5,
    RC6,
    LG,
    PANASONIC,
    RAW
};

/// Received IR code
struct IRCode {
    IRProtocol protocol;
    uint64_t code;
    uint16_t bits;
    String protocolName;
    uint16_t* rawData;      // For raw captures
    uint16_t rawLength;
};

/// IR Driver
class IRDriver {
public:
    static IRDriver& instance();

    void init();
    void update();

    // Transmit
    void send(IRProtocol protocol, uint64_t code, uint16_t bits = 32);
    void sendNEC(uint64_t code);
    void sendSamsung(uint64_t code);
    void sendSony(uint64_t code, uint16_t bits = 12);
    void sendRaw(const uint16_t* data, uint16_t length, uint16_t freq = 38);

    // Receive / Learning
    void startReceiver();
    void stopReceiver();
    bool isReceiving() const { return receiving_; }

    // Callback for received codes
    using ReceiveCallback = std::function<void(const IRCode& code)>;
    void onReceive(ReceiveCallback cb) { receiveCb_ = cb; }

private:
    IRDriver() = default;

    IRsend* sender_ = nullptr;
    IRrecv* receiver_ = nullptr;
    decode_results results_;
    bool receiving_ = false;
    ReceiveCallback receiveCb_ = nullptr;
};

// Global helper for CommandRouter
void ir_send(uint16_t code, uint8_t protocol);
