#include "MCP251x.hpp"
#include <thread>
#include <chrono>

#define RESET        0xC0
#define READ         0x03
#define WRITE        0x02
#define BIT_MODIFY   0x05
#define READ_STATUS  0xA0
#define RTS_TX0      0x81

MCP251x::MCP251x(SPI& spi) : spi_(spi) {}

void MCP251x::reset() {
    uint8_t cmd = RESET;
    spi_.write(&cmd, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void MCP251x::configure() {
    bitModify(0x0F, 0xE0, 0x80); 
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void MCP251x::setBitrate(uint8_t cnf1, uint8_t cnf2, uint8_t cnf3) {
    writeRegister(0x2A, cnf1); 
    writeRegister(0x29, cnf2); 
    writeRegister(0x28, cnf3); 
}

void MCP251x::setNormalMode() {
    bitModify(0x0F, 0xE0, 0x00); 
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void MCP251x::writeRegister(uint8_t address, uint8_t value) {
    std::vector<uint8_t> tx = {WRITE, address, value};
    spi_.write(tx.data(), tx.size());
}

uint8_t MCP251x::readRegister(uint8_t address) {
    std::vector<uint8_t> tx = {READ, address, 0x00};
    auto rx = spi_.transfer(tx);
    return rx[2];
}

void MCP251x::bitModify(uint8_t address, uint8_t mask, uint8_t data) {
    std::vector<uint8_t> tx = {BIT_MODIFY, address, mask, data};
    spi_.write(tx.data(), tx.size());
}

uint8_t MCP251x::readStatus() {
    std::vector<uint8_t> tx = {READ_STATUS, 0x00};
    auto rx = spi_.transfer(tx);
    return rx[1];
}

bool MCP251x::transmit(const std::array<uint8_t, 8>& data, uint8_t length, uint32_t can_id, bool extended) {
    if (length > 8) return false;

    std::vector<uint8_t> tx = {WRITE, 0x31};

    if (extended) {
        uint8_t sidh = (can_id >> 21) & 0xFF;
        uint8_t sidl = ((can_id >> 16) & 0x03) << 5;
        sidl |= (1 << 3); // IDE = 1
        sidl |= (can_id >> 18) & 0x07;
        uint8_t eid8 = (can_id >> 8) & 0xFF;
        uint8_t eid0 = can_id & 0xFF;

        tx.push_back(sidh);
        tx.push_back(sidl);
        tx.push_back(eid8);
        tx.push_back(eid0);
    } else {
        uint8_t sidh = (can_id >> 3) & 0xFF;
        uint8_t sidl = (can_id << 5) & 0xE0;

        tx.push_back(sidh);
        tx.push_back(sidl);
        tx.push_back(0x00);
        tx.push_back(0x00);
    }

    tx.push_back(length & 0x0F); // DLC
    tx.insert(tx.end(), data.begin(), data.begin() + length);

    spi_.write(tx.data(), tx.size());

    uint8_t rts = RTS_TX0;
    spi_.write(&rts, 1);

    return true;
}

bool MCP251x::receive(std::array<uint8_t, 8>& data, uint8_t& length, uint32_t& can_id) {
    uint8_t status = readStatus();
    if (!(status & 0x01)) return false; 

    std::vector<uint8_t> tx = {READ, 0x61}; 
    tx.resize(14, 0x00);

    auto rx = spi_.transfer(tx);

    uint8_t sidh = rx[2];
    uint8_t sidl = rx[3];
    uint8_t eid8 = rx[4];
    uint8_t eid0 = rx[5];
    length = rx[6] & 0x0F;

    if (sidl & (1 << 3)) {
        // Extended ID
        can_id = ((uint32_t)sidh << 21) |
                 ((uint32_t)(sidl & 0xE0) << 13) |
                 ((uint32_t)(sidl & 0x03) << 16) |
                 ((uint32_t)eid8 << 8) |
                 eid0;
    } else {
        // Standard ID
        can_id = ((uint16_t)sidh << 3) | (sidl >> 5);
    }

    for (uint8_t i = 0; i < length; ++i) {
        data[i] = rx[7 + i];
    }

    bitModify(0x2C, 0x01, 0x00); 

    return true;
}

void MCP251x::setRxMask(uint8_t maskNum, uint32_t mask) {
    uint8_t addr = (maskNum == 0) ? 0x20 : 0x24;

    uint8_t sidh = (mask >> 3) & 0xFF;
    uint8_t sidl = (mask << 5) & 0xE0;

    writeRegister(addr, sidh);
    writeRegister(addr + 1, sidl);
    writeRegister(addr + 2, 0x00);
    writeRegister(addr + 3, 0x00);
}

void MCP251x::setRxFilter(uint8_t filterNum, uint32_t id) {
    static const uint8_t baseAddrs[] = {0x00, 0x04, 0x08, 0x10, 0x14, 0x18};
    if (filterNum >= 6) return;

    uint8_t addr = baseAddrs[filterNum];

    uint8_t sidh = (id >> 3) & 0xFF;
    uint8_t sidl = (id << 5) & 0xE0;

    writeRegister(addr, sidh);
    writeRegister(addr + 1, sidl);
    writeRegister(addr + 2, 0x00);
    writeRegister(addr + 3, 0x00);
}

void MCP251x::onTransmitComplete(std::function<void()> callback) {
    txCallback_ = callback;
}

void MCP251x::onReceive(std::function<void(const std::array<uint8_t, 8>&, uint8_t, uint32_t)> callback) {
    rxCallback_ = callback;
}

void MCP251x::poll() {
    uint8_t ctrl = readRegister(0x30); 
    if (!(ctrl & 0x08) && txCallback_) {
        txCallback_();
        txCallback_ = nullptr;
    }

    uint8_t status = readStatus();
    if ((status & 0x01) && rxCallback_) {
        std::array<uint8_t, 8> data;
        uint8_t length;
        uint32_t can_id;

        if (receive(data, length, can_id)) {
            rxCallback_(data, length, can_id);
        }
    }
}
