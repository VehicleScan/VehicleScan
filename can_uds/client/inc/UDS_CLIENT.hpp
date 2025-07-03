#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <chrono>
#include "can_tp.hpp"

/*
*author Adham Walaa
*email adham.walaa@gmail.com
*/


/**
 * @class CAN_UDS_client
 * @brief Provides a high-level interface for UDS (Unified Diagnostic Services) communication over ISO-TP on CAN.
 *
 * This class uses a CAN_TP (ISO-TP over CAN) transport layer to send and receive UDS requests and interpret responses.
 */
class CAN_UDS_client {
public:
    /**
     * @brief Constructs a UDS client with specified CAN interface and IDs.
     * @param interface The name of the CAN interface (e.g., "can0").
     * @param tx_id Transmit CAN ID.
     * @param rx_id Receive CAN ID.
     */
    CAN_UDS_client(const std::string& interface, uint32_t tx_id, uint32_t rx_id);
    ~CAN_UDS_client() = default;

    /**
     * @enum UDS_NRC
     * @brief Defines possible UDS Negative Response Codes (NRC).
     */
    enum class UDS_NRC : uint8_t {
        NO_ERROR = 0x00,
        TIMEOUT = 0x01,
        GENERAL_REJECT = 0x10,
        SERVICE_NOT_SUPPORTED = 0x11,
        SUB_FUNCTION_NOT_SUPPORTED = 0x12,
        INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT = 0x13,
        RESPONSE_TOO_LONG = 0x14,
        BUSY_REPEAT_REQUEST = 0x21,
        CONDITIONS_NOT_CORRECT = 0x22,
        REQUEST_OUT_OF_RANGE = 0x31,
        SECURITY_ACCESS_DENIED = 0x33,
        INVALID_KEY = 0x35,
        EXCEED_NUMBER_OF_ATTEMPTS = 0x36,
        REQUIRED_TIME_DELAY_NOT_EXPIRED = 0x37
    };

    /**
     * @enum UDS_SESSION
     * @brief Enumerates the different diagnostic sessions defined by UDS.
     */
    enum class UDS_SESSION : uint8_t {
        DEFAULT_SESSION = 0x01,
        PROGRAMMING_SESSION = 0x02,
        EXTENDED_DIAGNOSTIC_SESSION = 0x03,
        SAFETY_SYSTEM_DIAGNOSTIC_SESSION = 0x04,
        VEHICLE_DIAGNOSTIC_SESSION = 0x05
    };

    /**
     * @enum UDS_SERVICE
     * @brief UDS service identifiers supported by this client.
     */
    enum class UDS_SERVICE : uint8_t {
        READ_DATA_BY_IDENTIFIER = 0x22,
        WRITE_DATA_BY_IDENTIFIER = 0x2E,
        READ_MEMORY_BY_ADDRESS = 0x23,
        WRITE_MEMORY_BY_ADDRESS = 0x3D,
        DIAGNOSTIC_SESSION_CONTROL = 0x10,
        ECU_RESET = 0x11,
        CLEAR_DIAGNOSTIC_INFORMATION = 0x14,
        READ_DTC_INFORMATION = 0x19,
        REQUEST_DOWNLOAD = 0x34,
        REQUEST_UPLOAD = 0x35,
        TRANSFER_DATA = 0x36,
        REQUEST_TRANSFER_EXIT = 0x37
    };

    /**
     * @struct UDS_MSG
     * @brief Represents a UDS message with service, payload, and response status.
     */
    struct UDS_MSG {
        UDS_SERVICE service;              ///< UDS service identifier
        std::vector<uint8_t> payload;     ///< Raw payload data
        UDS_NRC nrc = UDS_NRC::NO_ERROR;  ///< NRC from response, if any
        bool success = true;              ///< Indicates if the response was positive

        UDS_MSG(UDS_SERVICE s, const std::vector<uint8_t>& d)
            : service(s), payload(d) {}

        UDS_MSG() {
            service = UDS_SERVICE::READ_DATA_BY_IDENTIFIER;
            payload = std::vector<uint8_t>();
        }
    };

    /**
     * @brief Requests a diagnostic session change.
     * @param session Desired diagnostic session.
     * @param timeout Timeout for receiving a response.
     * @return NRC indicating the result of the request.
     */
    UDS_NRC set_session(UDS_SESSION session, std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));

    /**
     * @brief Sends a UDS request and waits for the response.
     * @param msg UDS request message.
     * @param timeout Timeout for receiving a response.
     * @return Parsed UDS_MSG with response data and NRC.
     */
    UDS_MSG send_request(const UDS_MSG& msg, std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));

    /**
     * @brief Sends a UDS request without waiting for a response.
     * @param msg UDS request message.
     */
    void send_request_no_response(const UDS_MSG& msg);

    /**
     * @brief Sets ISO-TP flow control options.
     * @param bs Block size (number of consecutive frames).
     * @param stmin Minimum separation time (in ms or STmin units).
     */
    void set_flow_control(uint32_t bs, uint32_t stmin);

private:
    std::mutex tx_mutex;                         ///< Mutex for thread-safe transmission
    UDS_SESSION currentSession = UDS_SESSION::DEFAULT_SESSION;
    std::shared_ptr<CAN_TP> can;                 ///< Transport protocol handler
    std::condition_variable queue_cv;            ///< Condition variable (not currently used)
    std::queue<std::vector<uint8_t>> request_queue; ///< Request queue (not currently used)
    std::thread receiver_thread;                 ///< Background thread for reception (not started)
    std::atomic<bool> running{false};            ///< Flag to manage background thread
};
