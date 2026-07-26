/**
 * ir_hal.cpp — IR Transmitter/Receiver Implementation
 * 
 * Based on OMOTE infrared_sender_hal_esp32.cpp with added learning mode
 */

#include "ir_hal.h"
#include "../config/pin_config.h"
#include "../config/user_config.h"

static IRsend irSender(IR_LED_GPIO, true);
static IRrecv *irReceiver = nullptr;
static decode_results irResults;
static IRLearnedCode lastLearnedCode = {UNKNOWN, 0, 0, false, ""};
static bool receiverActive = false;

// Default bits/repeat for common protocols
static void getDefaults(int protocol, uint16_t &bits, uint16_t &repeat) {
    switch (protocol) {
        case NEC:     bits = 32; repeat = 0; break;
        case SAMSUNG: bits = 32; repeat = 0; break;
        case SONY:    bits = 20; repeat = 2; break;
        case RC5:     bits = 13; repeat = 0; break;
        case RC6:     bits = 20; repeat = 0; break;
        case LG:      bits = 28; repeat = 0; break;
        case SHARP:   bits = 15; repeat = 0; break;
        case PANASONIC_AC: bits = 48; repeat = 0; break;
        default:      bits = 32; repeat = 0; break;
    }
}

void ir_init() {
    // TX init
    pinMode(IR_LED_GPIO, OUTPUT);
    digitalWrite(IR_LED_GPIO, HIGH); // HIGH = off
    irSender.begin();
    
    // RX init (power on receiver)
    pinMode(IR_VCC_GPIO, OUTPUT);
    digitalWrite(IR_VCC_GPIO, LOW); // Start with receiver off to save power
    
    irReceiver = new IRrecv(IR_RX_GPIO, 1024, 50, true);
    
    Serial.printf("[IR] TX on GPIO%d, RX on GPIO%d\n", IR_LED_GPIO, IR_RX_GPIO);
}

void ir_send(int protocol, uint64_t data, uint16_t bits, uint16_t repeat) {
    if (bits == 0 || repeat == 0) {
        uint16_t defBits, defRepeat;
        getDefaults(protocol, defBits, defRepeat);
        if (bits == 0) bits = defBits;
        if (repeat == 0) repeat = defRepeat;
    }
    
    Serial.printf("[IR] Sending protocol=%d data=0x%llX bits=%d repeat=%d\n",
                  protocol, data, bits, repeat);
    irSender.send((decode_type_t)protocol, data, bits, repeat);
}

void ir_send_raw(uint16_t* buf, uint16_t len) {
    Serial.printf("[IR] Sending RAW, %d elements\n", len);
    irSender.sendRaw(buf, len, 38); // 38kHz carrier
}

void ir_receiver_start() {
    if (!receiverActive) {
        digitalWrite(IR_VCC_GPIO, HIGH); // Power on receiver
        delay(50);
        irReceiver->enableIRIn();
        receiverActive = true;
        Serial.println("[IR] Receiver started (learning mode)");
    }
}

void ir_receiver_stop() {
    if (receiverActive) {
        irReceiver->disableIRIn();
        digitalWrite(IR_VCC_GPIO, LOW); // Power off receiver
        receiverActive = false;
        Serial.println("[IR] Receiver stopped");
    }
}

void ir_receiver_loop() {
    if (!receiverActive) return;
    
    if (irReceiver->decode(&irResults)) {
        if (irResults.decode_type != UNKNOWN && irResults.value != 0) {
            lastLearnedCode.protocol = irResults.decode_type;
            lastLearnedCode.code = irResults.value;
            lastLearnedCode.bits = irResults.bits;
            lastLearnedCode.valid = true;
            
            String protName = typeToString(irResults.decode_type);
            strncpy(lastLearnedCode.protocol_name, protName.c_str(), 23);
            lastLearnedCode.protocol_name[23] = '\0';
            
            Serial.printf("[IR] Learned: %s 0x%llX (%d bits)\n",
                         lastLearnedCode.protocol_name, 
                         lastLearnedCode.code, 
                         lastLearnedCode.bits);
        }
        irReceiver->resume();
    }
}

bool ir_receiver_available() {
    return lastLearnedCode.valid;
}

IRLearnedCode ir_receiver_get_last() {
    IRLearnedCode result = lastLearnedCode;
    lastLearnedCode.valid = false; // Clear after read
    return result;
}

// --- Convenience functions using user_config IR devices ---

void ir_send_power(int idx) {
    if (idx >= 0 && idx < NUM_IR_DEVICES)
        ir_send(IR_DEVICES[idx].protocol, IR_DEVICES[idx].power);
}

void ir_send_volume_up(int idx) {
    if (idx >= 0 && idx < NUM_IR_DEVICES)
        ir_send(IR_DEVICES[idx].protocol, IR_DEVICES[idx].vol_up);
}

void ir_send_volume_down(int idx) {
    if (idx >= 0 && idx < NUM_IR_DEVICES)
        ir_send(IR_DEVICES[idx].protocol, IR_DEVICES[idx].vol_down);
}

void ir_send_mute(int idx) {
    if (idx >= 0 && idx < NUM_IR_DEVICES)
        ir_send(IR_DEVICES[idx].protocol, IR_DEVICES[idx].mute);
}

void ir_send_channel_up(int idx) {
    if (idx >= 0 && idx < NUM_IR_DEVICES)
        ir_send(IR_DEVICES[idx].protocol, IR_DEVICES[idx].ch_up);
}

void ir_send_channel_down(int idx) {
    if (idx >= 0 && idx < NUM_IR_DEVICES)
        ir_send(IR_DEVICES[idx].protocol, IR_DEVICES[idx].ch_down);
}
