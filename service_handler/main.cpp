#include "UDS_SERVER.hpp"
#include <iostream>
#include <csignal>
#include <atomic>
#include <iomanip>

std::atomic<bool> keep_running(true);

void signal_handler(int signal) {
    if (signal == SIGINT) {
        keep_running = false;
    }
}

std::vector<uint8_t> read_data_handler(CAN_UDS_SERVER::UDS_SERVICE service, const std::vector<uint8_t>& request) {
    if (request.size() < 3 || (request.size() - 1) % 2 != 0) {
        return {
            0x7F,
            static_cast<uint8_t>(service),
            static_cast<uint8_t>(CAN_UDS_SERVER::UDS_NRC::INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT)
        };
    }

    std::vector<uint8_t> response;
    response.push_back(static_cast<uint8_t>(service) + 0x40);

    for (size_t i = 1; i + 1 < request.size(); i += 2) {
        uint8_t did_high = request[i];
        uint8_t did_low = request[i + 1];
        response.push_back(did_high);
        response.push_back(did_low);

        uint16_t did = (did_high << 8) | did_low;

        switch (did) {
            case 0x5678:  // RPM = 3120
                response.push_back(0x0C);
                response.push_back(0x30);
                break;
            case 0x1234:  // Speed = 100
                response.push_back(0x00);
                response.push_back(0x64);
                break;
            case 0xABCD:  // Oil Temp = 60°C
                response.push_back(0x3C);
                break;
            case 0x0123:  // Airflow = 256
                response.push_back(0x01);
                response.push_back(0x00);
                break;
            case 0x3456:  // Tire Pressure = 39 PSI
                response.push_back(0x27);
                break;
            default:
                return {
                    0x7F,
                    static_cast<uint8_t>(service),
                    static_cast<uint8_t>(CAN_UDS_SERVER::UDS_NRC::REQUEST_OUT_OF_RANGE)
                };
        }
    }

    return response;
}

int main() {
    std::signal(SIGINT, signal_handler);
    constexpr uint32_t CAN_TX_ID = 0x7E8; // ECU responds with this
    constexpr uint32_t CAN_RX_ID = 0x7DF; // ECU listens on this

    CAN_UDS_SERVER uds("can0", CAN_TX_ID, CAN_RX_ID);

    // Set flow control for ISO-TP
    uds.set_flow_control(16, 20);

    // Register service for READ_DATA_BY_IDENTIFIER
    uds.register_service(
        CAN_UDS_SERVER::UDS_SERVICE::READ_DATA_BY_IDENTIFIER,
        {
            CAN_UDS_SERVER::UDS_SESSION::DEFAULT_SESSION,
            CAN_UDS_SERVER::UDS_SESSION::EXTENDED_DIAGNOSTIC_SESSION
        },
        read_data_handler
    );

    uds.start();
    std::cout << "UDS Server started. Press Ctrl+C to stop.\n";

    while (keep_running) {
        try {
            std::vector<uint8_t> req = uds.receive_request();
            std::vector<uint8_t> resp = uds.handle_request(req);
            uds.send_response(resp);

            std::cout << "Request: ";
            for (uint8_t b : req) std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b) << " ";
            std::cout << "\nResponse: ";
            for (uint8_t b : resp) std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b) << " ";
            std::cout << std::dec << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
    }

    std::cout << "Stopping UDS server..." << std::endl;
    uds.stop();
    return 0;
}
