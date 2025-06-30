#include <MCP251x.hpp>
#include <SPI.hpp>
#include <UDS.hpp>
#include <iostream>
#include <thread>
#include <chrono>

int main(void){
    SPI::Config spi_config;
    spi_config.device = "/dev/spidev0.0"; // Adjust as necessary for your system
    spi_config.speed_hz = 1000000; // Set the SPI speed (1
    SPI spi(spi_config);
    MCP251x mcp251x(spi);
    UDS_ UDS_(mcp251x);

    mcp251x.reset();
    mcp251x.configure();
    mcp251x.setBitrate(0x03,0xFA,0x87); // Example bitrate settings
    mcp251x.setNormalMode();

    
    std::array<uint8_t, 8> data;
    uint8_t length = 0;
    uint32_t can_id = 0;

    UDS_.registerOnRequestReceivedCB([&](const UDS_::UDS__Msg& msg) {
        UDS_.sendPositiveResponse(msg.sid, {0x55, 0x02, 0x03});
    });

    while(true){
        if(UDS_.checkReceived(data, length, UDS_::CAN_ID::CLIENT_ID)){
            std::cout<<"received data "<<std::endl;
            for(int i=0;i<(int)data.size();i++){
                std::cout<<data[i]<<" ";
            }
            std::cout<<std::endl;
        } 
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); 
    }

    return 0;

}