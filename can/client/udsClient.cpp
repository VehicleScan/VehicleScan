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
    int counter = 0;
    while(true){
        std::array<uint8_t, 8> data = {
            uint8_t(counter & 0xFF), 0x01, 0x02, 0x03, 0x04, 0xAA, 0xBB, 0xCC
        };

        if (mcp251x.transmit(data, 8, 0x7FF, false)) {
            std::cout << "[TX] Sent frame #" << counter << "\n";
        } else {
            std::cerr << "[TX] Failed to send\n";
        }
        counter++;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}