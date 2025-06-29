#ifndef MCP251X_HPP
#define MCP251X_HPP

#include "SPI.hpp"
#include <array>
#include <functional>
#include <vector>

class MCP251x {
public:
    MCP251x(SPI& spi);

    void reset();
    void configure();
    void setBitrate(uint8_t cnf1, uint8_t cnf2, uint8_t cnf3);
    void setNormalMode();

    void writeRegister(uint8_t address, uint8_t value);
    uint8_t readRegister(uint8_t address);
    void bitModify(uint8_t address, uint8_t mask, uint8_t data);
    uint8_t readStatus();

    bool transmit(const std::array<uint8_t, 8>& data, uint8_t length, uint32_t can_id, bool extended);
    bool receive(std::array<uint8_t, 8>& data, uint8_t& length, uint32_t& can_id);

    void setRxMask(uint8_t maskNum, uint32_t mask);
    void setRxFilter(uint8_t filterNum, uint32_t id);

    void onTransmitComplete(std::function<void()> callback);
    void onReceive(std::function<void(const std::array<uint8_t, 8>&, uint8_t, uint32_t)> callback);

    void poll(); 

private:
    SPI& spi_;
    std::function<void()> txCallback_;
    std::function<void(const std::array<uint8_t, 8>&, uint8_t, uint32_t)> rxCallback_;
};
#endif // MCP251X_HPP