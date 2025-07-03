#pragma once

#include "can_tp.hpp"
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <functional>
#include <map>
#include <set>
#include <atomic>
#include <memory>
#include <vector>

/*
*author Adham Walaa
*email adham.walaa@gmail.com
*/


/**
 * @class CAN_UDS_SERVER
 * @brief UDS (Unified Diagnostic Services) server implementation using ISO-TP over CAN (SocketCAN).
 * 
 * This class handles incoming UDS requests from a CAN client, dispatches them to registered
 * service handlers, and sends appropriate responses. It supports session management and 
 * flow control configuration.
 */
class CAN_UDS_SERVER {
public:
    /// Supported UDS service identifiers
    enum class UDS_SERVICE : uint8_t {
        READ_DATA_BY_IDENTIFIER         = 0x22,
        WRITE_DATA_BY_IDENTIFIER        = 0x2E,
        DIAGNOSTIC_SESSION_CONTROL      = 0x10,
        ECU_RESET                       = 0x11,
        CLEAR_DIAGNOSTIC_INFORMATION    = 0x14,
        READ_DTC_INFORMATION            = 0x19,
        REQUEST_DOWNLOAD                = 0x34,
        REQUEST_UPLOAD                  = 0x35,
        TRANSFER_DATA                   = 0x36,
        REQUEST_TRANSFER_EXIT          = 0x37
    };

    /// Supported UDS diagnostic sessions
    enum class UDS_SESSION : uint8_t {
        DEFAULT_SESSION                 = 0x01,
        PROGRAMMING_SESSION            = 0x02,
        EXTENDED_DIAGNOSTIC_SESSION    = 0x03,
        SAFETY_SYSTEM_DIAGNOSTIC_SESSION = 0x04,
        VEHICLE_DIAGNOSTIC_SESSION     = 0x05
    };

    /// UDS negative response codes (NRCs)
    enum class UDS_NRC : uint8_t {
        NO_ERROR                        = 0x00,
        TIMEOUT                         = 0x01,
        GENERAL_REJECT                  = 0x10,
        SERVICE_NOT_SUPPORTED           = 0x11,
        SUB_FUNCTION_NOT_SUPPORTED      = 0x12,
        INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT = 0x13,
        RESPONSE_TOO_LONG               = 0x14,
        BUSY_REPEAT_REQUEST             = 0x21,
        CONDITIONS_NOT_CORRECT          = 0x22,
        REQUEST_OUT_OF_RANGE            = 0x31,
        SECURITY_ACCESS_DENIED          = 0x33,
        INVALID_KEY                     = 0x35,
        EXCEED_NUMBER_OF_ATTEMPTS       = 0x36,
        REQUIRED_TIME_DELAY_NOT_EXPIRED = 0x37,
        SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION = 0x7E
    };

    /**
     * @typedef ServiceHandler
     * @brief Function handler type for processing a UDS request.
     */
    using ServiceHandler = std::function<std::vector<uint8_t>(UDS_SERVICE, const std::vector<uint8_t>&)>;

    /**
     * @brief Constructor
     * @param interface CAN interface name (e.g., "can0")
     * @param tx_id Transmit CAN ID
     * @param rx_id Receive CAN ID
     */
    CAN_UDS_SERVER(const std::string& interface, uint32_t tx_id, uint32_t rx_id);

    /// Destructor, ensures threads are stopped
    ~CAN_UDS_SERVER();

    /// Starts the receiver loop thread
    void start();

    /// Stops the receiver loop thread
    void stop();

    /**
     * @brief Registers a UDS service handler.
     * @param service Service ID
     * @param allowed_sessions Allowed sessions for this service
     * @param handler Callback function to handle the request
     */
    void register_service(UDS_SERVICE service, const std::set<UDS_SESSION>& allowed_sessions, ServiceHandler handler);

    /**
     * @brief Waits for and returns the next incoming UDS request from the queue.
     */
    std::vector<uint8_t> receive_request();

    /**
     * @brief Sends a response back over CAN.
     * @param response Response payload
     */
    void send_response(const std::vector<uint8_t>& response);

    /**
     * @brief Sets flow control parameters for ISO-TP.
     * @param bs Block size
     * @param stmin Minimum separation time (ms)
     */
    void set_flow_control(uint32_t bs, uint32_t stmin);

    /**
     * @brief Dispatches a request to the appropriate handler based on service ID.
     * @param request Raw UDS request frame
     * @return UDS response frame
     */
    std::vector<uint8_t> handle_request(const std::vector<uint8_t>& request);

private:
    std::shared_ptr<CAN_TP> can; ///< ISO-TP CAN transport layer

    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    std::queue<std::vector<uint8_t>> request_queue;
    std::thread receiver_thread;
    std::atomic<bool> running{false};

    UDS_SESSION current_session = UDS_SESSION::DEFAULT_SESSION;

    std::map<UDS_SERVICE, std::pair<std::set<UDS_SESSION>, ServiceHandler>> service_handlers;

    /// Receiver thread loop function
    void receiver_loop();

    /// Handles built-in session control requests
    std::vector<uint8_t> handle_session_control(const std::vector<uint8_t>& request);

    static constexpr uint8_t NEGATIVE_RESPONSE_SID = 0x7F;
};
