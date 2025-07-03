#include "can_tp.hpp"
#include <iostream>
#include <thread>
#include <chrono>

void run_server() {
    CAN_TP server("can0", 0x321, 0x123);
    std::cout << "[SERVER] Waiting for message..." << std::endl;
    while (true) {
        auto data = server.receive();
        std::cout << "[SERVER] Received: ";
        for (auto b : data) std::cout << std::hex << (int)b << " ";
        std::cout << std::endl;

        // Echo back the same message
        server.send(data);
    }
}

void run_client() {
    std::this_thread::sleep_for(std::chrono::seconds(1));  // Let server start
    CAN_TP client("can0", 0x123, 0x321);

    std::vector<uint8_t> message = {0x02, 0x10, 0x03}; // Example UDS: Diagnostic Session Control
    std::cout << "[CLIENT] Sending request..." << std::endl;
    client.send(message);

    auto response = client.receive();
    std::cout << "[CLIENT] Got response: ";
    for (auto b : response) std::cout << std::hex << (int)b << " ";
    std::cout << std::endl;
}

int main() {
    std::thread server_thread(run_server);
    std::thread client_thread(run_client);

    client_thread.join();
    // Optionally end server thread after one exchange
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 0;
}
