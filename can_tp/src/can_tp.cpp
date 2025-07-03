#include "can_tp.hpp"

CAN_TP::CAN_TP(const std::string& interface, uint32_t tx_id, uint32_t rx_id)
    : ifname(interface), tx_id(tx_id), rx_id(rx_id) {
    
    // Create an ISO-TP socket
    sockfd = socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP);
    
    // Prepare interface request structure
    struct ifreq ifr;
    strncpy(ifr.ifr_name, ifname.c_str(), IFNAMSIZ);

    // Get the index of the CAN interface
    ioctl(sockfd, SIOCGIFINDEX, &ifr);

    // Bind the socket to the CAN interface and TX/RX IDs
    struct sockaddr_can addr = {
        .can_family = AF_CAN,
        .can_ifindex = ifr.ifr_ifindex,
        .can_addr = {
            .tp = {
                .rx_id = rx_id,
                .tx_id = tx_id
            }
        }
    };
    bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
}

CAN_TP::~CAN_TP() {
    if (sockfd != -1) {
        close(sockfd);
    }
}

void CAN_TP::send(const std::vector<uint8_t>& payload) {
    write(sockfd, payload.data(), payload.size());
}

void CAN_TP::set_receive_timeout(std::chrono::milliseconds timeout) {
    struct timeval tv;
    tv.tv_sec = timeout.count() / 1000;
    tv.tv_usec = (timeout.count() % 1000) * 1000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        throw std::system_error(errno, std::generic_category(), "setsockopt failed");
    }
}

std::vector<uint8_t> CAN_TP::receive() {
    std::vector<uint8_t> buffer(4095);  // ISO-TP allows up to 4095 bytes
    ssize_t nbytes = read(sockfd, buffer.data(), buffer.size());

    if (nbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            throw std::runtime_error("CAN receive timeout");
        }
        throw std::system_error(errno, std::generic_category(), "CAN receive failed");
    }

    buffer.resize(nbytes);  // Trim buffer to actual received size
    return buffer;
}

std::vector<uint8_t> CAN_TP::send_and_receive(const std::vector<uint8_t>& payload, std::chrono::milliseconds timeout) {
    set_receive_timeout(timeout);
    send(payload);
    return receive();
}

int CAN_TP::get_sockfd() const {
    return sockfd;
}

void CAN_TP::set_flow_control(uint32_t bs, uint32_t stmin) {
    struct can_isotp_fc_options fc_opts = {
        .bs = static_cast<uint8_t>(bs),
        .stmin = static_cast<uint8_t>(stmin),
        .wftmax = 0,  // Wait frame transmissions (unused here)
    };
    setsockopt(sockfd, SOL_CAN_ISOTP, CAN_ISOTP_OPTS, &fc_opts, sizeof(fc_opts));
}
