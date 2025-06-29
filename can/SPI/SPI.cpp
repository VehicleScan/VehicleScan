#include "SPI.hpp"

SPI::SPI(const Config& config) : config_(config) {
    open_device();
    configure_device();
}

SPI::SPI(SPI&& other) noexcept : fd_(other.fd_), config_(other.config_) {
    other.fd_ = -1;
}

SPI::~SPI() {
    if (fd_ >= 0) {
        close(fd_);
    }
}

std::vector<uint8_t> SPI::transfer(const uint8_t* tx_data, size_t length) {
    std::vector<uint8_t> rx_data(length);
    
    spi_ioc_transfer tr = {
        .tx_buf = reinterpret_cast<uint64_t>(tx_data),
        .rx_buf = reinterpret_cast<uint64_t>(rx_data.data()),
        .len = static_cast<uint32_t>(length),
        .speed_hz = config_.speed_hz,
        .delay_usecs = config_.delay_usecs,
        .bits_per_word = config_.bits_per_word,
    };

    int status = ioctl(fd_, SPI_IOC_MESSAGE(1), &tr);
    if (status < 0) {
        throw std::runtime_error("SPI transfer failed: " + std::string(strerror(errno)));
    }

    return rx_data;
}

std::vector<uint8_t> SPI::transfer(const std::vector<uint8_t>& tx_data) {
    return transfer(tx_data.data(), tx_data.size());
}

void SPI::write(const uint8_t* data, size_t length) {
    if (::write(fd_, data, length) != static_cast<ssize_t>(length)) {
        throw std::runtime_error("SPI write failed: " + std::string(strerror(errno)));
    }
}

std::vector<uint8_t> SPI::read(size_t length) {
    std::vector<uint8_t> rx_data(length);
    if (::read(fd_, rx_data.data(), length) != static_cast<ssize_t>(length)) {
        throw std::runtime_error("SPI read failed: " + std::string(strerror(errno)));
    }
    return rx_data;
}

void SPI::set_mode(uint8_t mode) {
    config_.mode = mode;
    if (ioctl(fd_, SPI_IOC_WR_MODE, &config_.mode) < 0) {
        throw std::runtime_error("Failed to set SPI mode: " + std::string(strerror(errno)));
    }
}

void SPI::set_speed(uint32_t speed_hz) {
    config_.speed_hz = speed_hz;
    if (ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &config_.speed_hz) < 0) {
        throw std::runtime_error("Failed to set SPI speed: " + std::string(strerror(errno)));
    }
}

void SPI::set_bits_per_word(uint8_t bits) {
    config_.bits_per_word = bits;
    if (ioctl(fd_, SPI_IOC_WR_BITS_PER_WORD, &config_.bits_per_word) < 0) {
        throw std::runtime_error("Failed to set bits per word: " + std::string(strerror(errno)));
    }
}

void SPI::open_device() {
    fd_ = open(config_.device, O_RDWR);
    if (fd_ < 0) {
        throw std::runtime_error("Failed to open SPI device: " + std::string(strerror(errno)));
    }
}

void SPI::configure_device() {
    if (ioctl(fd_, SPI_IOC_WR_MODE, &config_.mode) < 0) {
        close(fd_);
        throw std::runtime_error("Failed to set SPI mode: " + std::string(strerror(errno)));
    }

    if (ioctl(fd_, SPI_IOC_WR_BITS_PER_WORD, &config_.bits_per_word) < 0) {
        close(fd_);
        throw std::runtime_error("Failed to set bits per word: " + std::string(strerror(errno)));
    }

    if (ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &config_.speed_hz) < 0) {
        close(fd_);
        throw std::runtime_error("Failed to set SPI speed: " + std::string(strerror(errno)));
    }
}