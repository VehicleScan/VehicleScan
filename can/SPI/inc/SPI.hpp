#ifndef SPI_HPP
#define SPI_HPP
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstring>
#include <errno.h>


class SPI {
public:
    struct Config {
        const char* device = "/dev/spidev0.0";
        uint8_t mode = SPI_MODE_0;
        uint8_t bits_per_word = 8;
        uint32_t speed_hz = 1000000;  
        uint16_t delay_usecs = 0;
    };

    explicit SPI(const Config& config);
    SPI(SPI&& other) noexcept;
    SPI(const SPI&) = delete;
    SPI& operator=(const SPI&) = delete;
    ~SPI();

    std::vector<uint8_t> transfer(const uint8_t* tx_data, size_t length);
    std::vector<uint8_t> transfer(const std::vector<uint8_t>& tx_data);
    void write(const uint8_t* data, size_t length);
    std::vector<uint8_t> read(size_t length);

    void set_mode(uint8_t mode);
    void set_speed(uint32_t speed_hz);
    void set_bits_per_word(uint8_t bits);

private:
    int fd_ = -1;
    Config config_;

    void open_device();
    void configure_device();
};

#endif 