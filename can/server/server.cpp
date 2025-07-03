#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include "MCP251x.hpp"

int main(void){
    SPI::Config config;
    config.device = "/dev/spidev0.0";
    config.speed_hz = 10000000;
    config.mode = SPI_MODE_0;

    SPI spi(config);
    MCP251x mcp251x(spi);

    mcp251x.reset();
    mcp251x.configure();
    mcp251x.setBitrate(0x03, 0x1C, 0x05); // Example bitrate settings
    mcp251x.setNormalMode();

    std::cout << "MCP251x initialized and configured." << std::endl;

    while(true){
        std::array<uint8_t, 8> data = {0};
        uint8_t length = 0;
        uint32_t can_id = 0x0; // Example CAN ID

        if (mcp251x.receive(data, length, can_id)) {
            std::cout << "Received CAN frame: ID=" << can_id << ", Length=" << static_cast<int>(length) << ", Data=";
            for (uint8_t i = 0; i < length; ++i) {
                std::cout << std::hex << static_cast<int>(data[i]) << " ";
            }
            std::cout << "can id: " << can_id; 
            std::cout << std::dec << std::endl;
        } 
        else {

        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


    return 0;
}


