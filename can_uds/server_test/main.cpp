#include "UDS_SERVER.hpp"
#include <iostream>
#include <thread>
#include <csignal>

// Signal flag for graceful shutdown
volatile std::sig_atomic_t signal_received = 0;

void signal_handler(int signal) {
    signal_received = signal;
}

int main() {
    // Set up signal handling
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // Create UDS server
    CAN_UDS_SERVER server("can0", 0x7E8, 0x7E0);
    
    // Register ReadDataByIdentifier service
    server.register_service(CAN_UDS_SERVER:: UDS_SERVICE::READ_DATA_BY_IDENTIFIER,
                           {CAN_UDS_SERVER::UDS_SESSION::DEFAULT_SESSION, CAN_UDS_SERVER::UDS_SESSION::EXTENDED_DIAGNOSTIC_SESSION},
                           [](CAN_UDS_SERVER::UDS_SERVICE service, const std::vector<uint8_t>& request) -> std::vector<uint8_t> {
        // Minimum request size: SID + 2-byte identifier
        if (request.size() < 3) {
            return {0x7F, 
                    static_cast<uint8_t>(service), 
                    static_cast<uint8_t>(CAN_UDS_SERVER::UDS_NRC::INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT)};
        }
        
        // Extract identifier (big-endian)
        uint16_t identifier = (request[1] << 8) | request[2];
        
        // Define constant values for identifiers
        std::vector<uint8_t> response;
        response.push_back(static_cast<uint8_t>(service) + 0x40);  // Positive response SID
        response.push_back(request[1]);  // Identifier high byte
        response.push_back(request[2]);  // Identifier low byte
        
        switch (identifier) {
            case 0xF190:
                // Return 4 bytes for identifier F190
                response.insert(response.end(), {0x01, 0x02, 0x03, 0x04});
                break;
            case 0x1234:
                // Return 2 bytes for identifier 1234
                response.insert(response.end(), {0xAA, 0xBB});
                break;
            case 0x5678:
                // Return 3 bytes for identifier 5678
                response.insert(response.end(), {0x11, 0x22, 0x33});
                break;
            default:
                // Identifier not supported
                return {0x7F, 
                        static_cast<uint8_t>(service), 
                        static_cast<uint8_t>(CAN_UDS_SERVER::UDS_NRC::REQUEST_OUT_OF_RANGE)};
        }
        
        return response;
    });
    
    // Start the server
    server.start();
    std::cout << "UDS Server started. Press Ctrl+C to exit..." << std::endl;
    
    // Main loop
    while (!signal_received) {
        auto request = server.receive_request();
        auto response = server.handle_request(request);
        server.send_response(response);
        
        // Log request/response
        std::cout << "Handled request: ";
        for (auto b : request) printf("%02X ", b);
        std::cout << "\nResponse: ";
        for (auto b : response) printf("%02X ", b);
        std::cout << "\n" << std::endl;
    }
    
    // Stop the server
    server.stop();
    std::cout << "UDS Server stopped." << std::endl;
    
    return 0;
}