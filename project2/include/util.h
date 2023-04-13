#pragma once

#include <cstddef>
#include <cstdint>
#include <cinttypes>
#include <iostream>
#include <stdexcept>
#include <string>
#include <array>
#include <sys/socket.h>

constexpr std::size_t k_max_eth_len = 1472;

template<typename T>
T parse_int(const std::string& s) {
    T port;
    try {
        port = static_cast<T>(std::stoul(s));
    } catch(const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        std::exit(1);
    }
    return port;
}

class UnreliableSocket {
public:
    UnreliableSocket();
    virtual ~UnreliableSocket();

    ssize_t u_bind(uint16_t port);
    ssize_t u_sendto(uint8_t* data, int data_len, const struct sockaddr* dest_addr, socklen_t addrlen);
    ssize_t u_recvfrom(uint8_t* data, int data_len, struct sockaddr* src_addr, socklen_t *addrlen);

protected:
    int _fd;
    std::array<uint8_t, 1024 * 2> _buf;
};
