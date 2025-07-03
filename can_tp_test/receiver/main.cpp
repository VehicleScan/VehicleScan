#include "can_tp.hpp"
#include <iostream>
#include <vector>

int main() {
    CAN_TP receiver("can0", 0x321, 0x123);  // TX ID = 0x321, RX ID = 0x123

    std::cout << "Waiting for ISO-TP message..." << std::endl;
    auto data = receiver.receive();

    std::string received_str(data.begin(), data.end());
    std::cout << "Received: " << received_str << std::endl;

    return 0;
}
