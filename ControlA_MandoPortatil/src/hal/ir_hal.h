/**
 * ir_hal.h — Infrared Transmitter/Receiver HAL
 * 
 * Multi-protocol IR support using IRremoteESP8266
 * Features: Send, receive, learn mode
 */

#pragma once

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>

// IR Learning result
struct IRLearnedCode {
    decode_type_t protocol;
    uint64_t code;
    uint16_t bits;
    bool valid;
    char protocol_name[24];
};

void ir_init();
void ir_send(int protocol, uint64_t data, uint16_t bits = 0, uint16_t repeat = 0);
void ir_send_raw(uint16_t* buf, uint16_t len);

// IR Receiver
void ir_receiver_start();
void ir_receiver_stop();
void ir_receiver_loop();
bool ir_receiver_available();
IRLearnedCode ir_receiver_get_last();

// Convenience functions
void ir_send_power(int device_index);
void ir_send_volume_up(int device_index);
void ir_send_volume_down(int device_index);
void ir_send_mute(int device_index);
void ir_send_channel_up(int device_index);
void ir_send_channel_down(int device_index);
