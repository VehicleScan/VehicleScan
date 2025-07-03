#pragma once

#include <linux/can/isotp.h>    // For CAN ISO-TP socket definitions
#include <string>
#include <vector>
#include <stdexcept>
#include <unistd.h>             // For close(), read(), write()
#include <sys/socket.h>         // For socket-related functions
#include <sys/ioctl.h>          // For ioctl(), used to get interface index
#include <net/if.h>             // For struct ifreq
#include <cstring>
#include <chrono>
#include <system_error>

/*
*author Adham Walaa
*email adham.walaa@gmail.com
*/

/**
 * @class CAN_TP
 * @brief A C++ wrapper around the Linux CAN ISO-TP socket API for sending and receiving transport protocol (ISO-TP) messages.
 *
 * This class simplifies communication over the CAN network using the ISO-TP protocol (used in UDS, OBD, etc.).
 */
class CAN_TP {
    int sockfd = -1;              ///< Socket file descriptor
    std::string ifname;           ///< CAN interface name (e.g., "can0")
    uint32_t tx_id;               ///< CAN ID for transmitting
    uint32_t rx_id;               ///< CAN ID for receiving

public:
    /**
     * @brief Constructor - Initializes and binds the CAN ISO-TP socket.
     * @param interface Name of the CAN interface (e.g., "can0")
     * @param tx_id Transmit CAN ID
     * @param rx_id Receive CAN ID
     */
    CAN_TP(const std::string& interface, uint32_t tx_id, uint32_t rx_id);

    /**
     * @brief Destructor - Closes the socket.
     */
    ~CAN_TP();

    /**
     * @brief Send a payload over ISO-TP.
     * @param payload Vector of bytes to send.
     */
    void send(const std::vector<uint8_t>& payload);

    /**
     * @brief Receive an ISO-TP frame (blocking or timeout).
     * @return Vector containing received data.
     * @throws std::runtime_error if timeout occurs.
     * @throws std::system_error on read failure.
     */
    std::vector<uint8_t> receive(); 

    /**
     * @brief Sends a payload and waits for the response with a timeout.
     * @param payload Data to send.
     * @param timeout Max time to wait for response.
     * @return Response received.
     */
    std::vector<uint8_t> send_and_receive(const std::vector<uint8_t>& payload, std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));

    /**
     * @brief Set timeout duration for receiving.
     * @param timeout Timeout duration in milliseconds.
     */
    void set_receive_timeout(std::chrono::milliseconds timeout);

    /**
     * @brief Configure ISO-TP flow control options.
     * @param bs Block Size (number of consecutive frames before flow control frame)
     * @param stmin Minimum separation time between frames (in milliseconds or ISO-TP units)
     */
    void set_flow_control(uint32_t bs = 8, uint32_t stmin = 0x0A);

    /**
     * @brief Get the socket file descriptor.
     * @return The socket file descriptor.
     */
    int get_sockfd() const;

    /**
     * @brief Lists available CAN interfaces on the system.
     * @return Vector of interface names (not implemented here).
     */
    static std::vector<std::string> list_interfaces();
};
