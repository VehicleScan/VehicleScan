#include "UDS_CLIENT.hpp"

CAN_UDS_client::CAN_UDS_client(const std::string& interface, uint32_t tx_id, uint32_t rx_id) {
    can = std::make_shared<CAN_TP>(interface, tx_id, rx_id);
}

CAN_UDS_client::UDS_NRC CAN_UDS_client::set_session(UDS_SESSION session, std::chrono::milliseconds timeout) {
    std::lock_guard<std::mutex> lock(tx_mutex);
    UDS_NRC result = UDS_NRC::GENERAL_REJECT;

    try {
        std::vector<uint8_t> request = {
            static_cast<uint8_t>(UDS_SERVICE::DIAGNOSTIC_SESSION_CONTROL),
            static_cast<uint8_t>(session)
        };

        std::vector<uint8_t> response = can->send_and_receive(request, timeout);

        if (response.size() >= 2 &&
            response[0] == static_cast<uint8_t>(UDS_SERVICE::DIAGNOSTIC_SESSION_CONTROL) + 0x40 &&
            response[1] == static_cast<uint8_t>(session)) {
            
            currentSession = session;
            result = UDS_NRC::NO_ERROR;

        } else if (response.size() >= 3 && response[0] == 0x7F &&
                   response[1] == static_cast<uint8_t>(UDS_SERVICE::DIAGNOSTIC_SESSION_CONTROL)) {
            result = static_cast<UDS_NRC>(response[2]);
        }
    } catch (const std::runtime_error& e) {
        if (std::string(e.what()) == "CAN receive timeout") {
            result = UDS_NRC::TIMEOUT;
        }
    }

    return result;
}

void CAN_UDS_client::set_flow_control(uint32_t bs, uint32_t stmin) {
    can->set_flow_control(bs, stmin);
}

CAN_UDS_client::UDS_MSG CAN_UDS_client::send_request(const UDS_MSG& msg, std::chrono::milliseconds timeout) {
    std::lock_guard<std::mutex> lock(tx_mutex);
    UDS_MSG result;
    result.service = msg.service;
    result.success = false;
    result.nrc = UDS_NRC::GENERAL_REJECT;

    try {
        std::vector<uint8_t> request = {
            static_cast<uint8_t>(msg.service)
        };
        request.insert(request.end(), msg.payload.begin(), msg.payload.end());

        std::vector<uint8_t> response = can->send_and_receive(request, timeout);

        if (response.size() >= 1 &&
            response[0] == static_cast<uint8_t>(msg.service) + 0x40) {
            
            result.success = true;
            result.nrc = UDS_NRC::NO_ERROR;

            if (response.size() > 1) {
                result.payload = std::vector<uint8_t>(response.begin() + 1, response.end());
            }
        } else if (response.size() >= 3 && response[0] == 0x7F &&
                   response[1] == static_cast<uint8_t>(msg.service)) {
            
            result.nrc = static_cast<UDS_NRC>(response[2]);
        }
    } catch (const std::runtime_error& e) {
        if (std::string(e.what()) == "CAN receive timeout") {
            result.nrc = UDS_NRC::TIMEOUT;
        }
    }

    return result;
}

void CAN_UDS_client::send_request_no_response(const UDS_MSG& msg) {
    std::lock_guard<std::mutex> lock(tx_mutex);
    std::vector<uint8_t> request = {
        static_cast<uint8_t>(msg.service)
    };
    request.insert(request.end(), msg.payload.begin(), msg.payload.end());
    can->send(request);
}
