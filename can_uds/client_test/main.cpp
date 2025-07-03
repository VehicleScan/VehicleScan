#include "UDS_CLIENT.hpp"
#include <iostream>
#include <iomanip>
#include <csignal>
#include <thread>

volatile std::sig_atomic_t signal_received = 0;

void signal_handler(int signal) {
    signal_received = signal;
}

void print_uds_message(const CAN_UDS_client::UDS_MSG& msg) {
    std::cout << "Service: 0x" << std::hex << std::setw(2) << std::setfill('0') 
              << static_cast<int>(msg.service);
    std::cout << " | Success: " << std::boolalpha << msg.success;
    
    if (msg.success) {
        std::cout << " | Payload: ";
        for (auto b : msg.payload) {
            std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') 
                      << static_cast<int>(b) << " ";
        }
    } else {
        std::cout << " | NRC: 0x" << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(msg.nrc);
    }
    std::cout << std::dec << std::endl;
}

int main() {
    std::signal(SIGINT, signal_handler);
    
    // Create UDS client (TX: 0x7E0, RX: 0x7E8)
    CAN_UDS_client client("can0", 0x7E0, 0x7E8);
    
    // Set flow control parameters (optional)
    client.set_flow_control(16, 20);  // BS=16, STmin=20ms
    
    std::cout << "UDS Client Test Application\n";
    std::cout << "==========================\n";
    
    std::cout << "Current session: Default Session\n";
    
    // Test 2: Change to extended session
    std::cout << "\n[Test 2] Changing to extended diagnostic session...\n";
    auto rc = client.set_session(CAN_UDS_client::UDS_SESSION::EXTENDED_DIAGNOSTIC_SESSION);
    if (rc == CAN_UDS_client::UDS_NRC::NO_ERROR) {
        std::cout << "Session changed successfully!\n";
    } else {
        std::cout << "Session change failed! NRC: 0x" << std::hex 
                  << static_cast<int>(rc) << std::dec << std::endl;
    }
    
    // Test 3: Read valid identifiers
    std::cout << "\n[Test 3] Reading valid identifiers...\n";
    
    // Identifier: 0xF190 (should return 4 bytes)
    {
        std::vector<uint8_t> payload = {0xF1, 0x90};
        CAN_UDS_client::UDS_MSG request{
            CAN_UDS_client::UDS_SERVICE::READ_DATA_BY_IDENTIFIER,
            payload
        };
        
        auto response = client.send_request(request);
        std::cout << "Read 0xF190: ";
        print_uds_message(response);
    }
    
    // Identifier: 0x1234 (should return 2 bytes)
    {
        std::vector<uint8_t> payload = {0x12, 0x34};
        CAN_UDS_client::UDS_MSG request{
            CAN_UDS_client::UDS_SERVICE::READ_DATA_BY_IDENTIFIER,
            payload
        };
        
        auto response = client.send_request(request);
        std::cout << "Read 0x1234: ";
        print_uds_message(response);
    }
    
    // Identifier: 0x5678 (should return 3 bytes)
    {
        std::vector<uint8_t> payload = {0x56, 0x78};
        CAN_UDS_client::UDS_MSG request{
            CAN_UDS_client::UDS_SERVICE::READ_DATA_BY_IDENTIFIER,
            payload
        };
        
        auto response = client.send_request(request);
        std::cout << "Read 0x5678: ";
        print_uds_message(response);
    }
    
    // Test 4: Read invalid identifier
    std::cout << "\n[Test 4] Reading invalid identifier...\n";
    {
        std::vector<uint8_t> payload = {0x99, 0x88};  // Unsupported ID
        CAN_UDS_client::UDS_MSG request{
            CAN_UDS_client::UDS_SERVICE::READ_DATA_BY_IDENTIFIER,
            payload
        };
        
        auto response = client.send_request(request);
        std::cout << "Read 0x9988: ";
        print_uds_message(response);
    }
    
    // Test 5: Invalid service in current session
    std::cout << "\n[Test 5] Requesting invalid service...\n";
    {
        // Attempt to request Programming Session service (0x10) with wrong subfunction
        std::vector<uint8_t> payload = {0x99};  // Invalid subfunction
        CAN_UDS_client::UDS_MSG request{
            CAN_UDS_client::UDS_SERVICE::DIAGNOSTIC_SESSION_CONTROL,
            payload
        };
        
        auto response = client.send_request(request);
        std::cout << "Invalid session control: ";
        print_uds_message(response);
    }
    
    // Test 6: Send request without waiting for response
    std::cout << "\n[Test 6] Sending request without response...\n";
    {
        // This would normally be a service that doesn't require response
        std::vector<uint8_t> payload = {0x01, 0x02, 0x03};
        CAN_UDS_client::UDS_MSG request{
            CAN_UDS_client::UDS_SERVICE::ECU_RESET,
            payload
        };
        
        client.send_request_no_response(request);
        std::cout << "Request sent (no response expected)\n";
    }
    
    std::cout << "\nAll tests completed!\n";
    return 0;
}