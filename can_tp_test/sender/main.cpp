#include "can_tp.hpp"
#include <iostream>
#include <vector>
#include <string>

int main() {
    CAN_TP sender("can0", 0x123, 0x321);  // TX ID = 0x123, RX ID = 0x321

    std::string message = "Hello from sender!";
    std::vector<uint8_t> data(message.begin(), message.end());

    std::cout << "Sending: " << message << std::endl;
    sender.send(data);

    return 0;
}
