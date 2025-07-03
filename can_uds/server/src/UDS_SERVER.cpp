#include "UDS_SERVER.hpp"
#include <sys/time.h>
#include <unistd.h>
#include <stdexcept>

CAN_UDS_SERVER::CAN_UDS_SERVER(const std::string& interface, uint32_t tx_id, uint32_t rx_id){
    register_service(UDS_SERVICE::DIAGNOSTIC_SESSION_CONTROL,
        {UDS_SESSION::DEFAULT_SESSION, UDS_SESSION::PROGRAMMING_SESSION, UDS_SESSION::EXTENDED_DIAGNOSTIC_SESSION},
        [this](UDS_SERVICE, const auto& req) { return handle_session_control(req); });
    can = std::make_shared<CAN_TP>(interface, tx_id, rx_id);
}

CAN_UDS_SERVER::~CAN_UDS_SERVER() {
    stop();
}

void CAN_UDS_SERVER::start() {
    if (running) return;
    running = true;
    receiver_thread = std::thread(&CAN_UDS_SERVER::receiver_loop, this);
}

void CAN_UDS_SERVER::stop() {
    running = false;
    if (receiver_thread.joinable()) {
        receiver_thread.join();
    }
}

void CAN_UDS_SERVER::receiver_loop() {
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(can->get_sockfd(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    std::vector<uint8_t> buffer(4095);
    while (running) {
        ssize_t nbytes = read(can->get_sockfd(), buffer.data(), buffer.size());
        if (nbytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            break;
        }

        std::vector<uint8_t> request(buffer.begin(), buffer.begin() + nbytes);
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            request_queue.push(request);
        }
        queue_cv.notify_one();
    }
}

void CAN_UDS_SERVER::register_service(UDS_SERVICE service, 
    const std::set<UDS_SESSION>& allowed_sessions, 
    ServiceHandler handler) {
    service_handlers[service] = {allowed_sessions, handler};
}

std::vector<uint8_t> CAN_UDS_SERVER::receive_request() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    queue_cv.wait(lock, [this] { return !request_queue.empty(); });
    
    auto request = request_queue.front();
    request_queue.pop();
    return request;
}

void CAN_UDS_SERVER::send_response(const std::vector<uint8_t>& response) {
    can->send(response);
}

std::vector<uint8_t> CAN_UDS_SERVER::handle_session_control(const std::vector<uint8_t>& request) {
    if (request.size() < 2) {
        return {NEGATIVE_RESPONSE_SID, 
                static_cast<uint8_t>(UDS_SERVICE::DIAGNOSTIC_SESSION_CONTROL), 
                static_cast<uint8_t>(UDS_NRC::INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT)};
    }
    
    UDS_SESSION new_session = static_cast<UDS_SESSION>(request[1]);
    current_session = new_session;
    return {static_cast<uint8_t>(UDS_SERVICE::DIAGNOSTIC_SESSION_CONTROL) + 0x40, 
            static_cast<uint8_t>(new_session)};
}

std::vector<uint8_t> CAN_UDS_SERVER::handle_request(const std::vector<uint8_t>& request) {
    if (request.empty()) {
        return {NEGATIVE_RESPONSE_SID, 0x00, static_cast<uint8_t>(UDS_NRC::GENERAL_REJECT)};
    }
    
    UDS_SERVICE service = static_cast<UDS_SERVICE>(request[0]);
    
    if (service == UDS_SERVICE::DIAGNOSTIC_SESSION_CONTROL) {
        return handle_session_control(request);
    }
    
    auto it = service_handlers.find(service);
    if (it != service_handlers.end()) {
        auto& [allowed_sessions, handler] = it->second;
        if (allowed_sessions.find(current_session) != allowed_sessions.end()) {
            return handler(service, request);
        }
        return {NEGATIVE_RESPONSE_SID, 
                static_cast<uint8_t>(service), 
                static_cast<uint8_t>(UDS_NRC::SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION)};
    }
    
    return {NEGATIVE_RESPONSE_SID, 
            static_cast<uint8_t>(service), 
            static_cast<uint8_t>(UDS_NRC::SERVICE_NOT_SUPPORTED)};
}

void CAN_UDS_SERVER::set_flow_control(uint32_t bs, uint32_t stmin) {
    can->set_flow_control(bs, stmin);
}