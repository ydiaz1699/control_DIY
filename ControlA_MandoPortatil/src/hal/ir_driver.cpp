#include "ir_driver.h"
#include "../config.h"

IRDriver& IRDriver::instance() {
    static IRDriver inst;
    return inst;
}

void IRDriver::init() {
    sender_ = new IRsend(Pins::IR_TX);
    sender_->begin();

    receiver_ = new IRrecv(Pins::IR_RX);
}

void IRDriver::update() {
    if (!receiving_) return;

    if (receiver_->decode(&results_)) {
        IRCode code;
        code.protocol = (IRProtocol)results_.decode_type;
        code.code = results_.value;
        code.bits = results_.bits;
        code.protocolName = typeToString(results_.decode_type);
        code.rawData = nullptr;
        code.rawLength = 0;

        if (receiveCb_) {
            receiveCb_(code);
        }

        receiver_->resume();
    }
}

void IRDriver::send(IRProtocol protocol, uint64_t code, uint16_t bits) {
    switch (protocol) {
        case IRProtocol::NEC:
            sender_->sendNEC(code, bits);
            break;
        case IRProtocol::SAMSUNG:
            sender_->sendSAMSUNG(code, bits);
            break;
        case IRProtocol::SONY:
            sender_->sendSony(code, bits);
            break;
        case IRProtocol::RC5:
            sender_->sendRC5(code, bits);
            break;
        case IRProtocol::RC6:
            sender_->sendRC6(code, bits);
            break;
        case IRProtocol::LG:
            sender_->sendLG(code, bits);
            break;
        case IRProtocol::PANASONIC:
            sender_->sendPanasonic(0x4004, code, bits);
            break;
        default:
            sender_->sendNEC(code, bits);
            break;
    }
}

void IRDriver::sendNEC(uint64_t code) {
    sender_->sendNEC(code);
}

void IRDriver::sendSamsung(uint64_t code) {
    sender_->sendSAMSUNG(code);
}

void IRDriver::sendSony(uint64_t code, uint16_t bits) {
    sender_->sendSony(code, bits);
}

void IRDriver::sendRaw(const uint16_t* data, uint16_t length, uint16_t freq) {
    sender_->sendRaw(data, length, freq);
}

void IRDriver::startReceiver() {
    receiving_ = true;
    receiver_->enableIRIn();
}

void IRDriver::stopReceiver() {
    receiving_ = false;
    receiver_->disableIRIn();
}

// Global helper
void ir_send(uint16_t code, uint8_t protocol) {
    IRDriver::instance().send((IRProtocol)protocol, (uint64_t)code);
}
