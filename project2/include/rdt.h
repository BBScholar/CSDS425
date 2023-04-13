#pragma once 

#include <array>
#include <cstdint>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <vector>

#include <cinttypes>
#include <memory>
#include <chrono>
#include <optional>

#include "crc.h"
#include "util.h"

using namespace std::chrono_literals;

enum HeaderType : uint8_t {
    Start = 0, End, Data, Ack, None
};

struct RDTHeader {
    uint8_t type;
    uint16_t length, checksum;
    uint32_t seq_num; 
};

constexpr uint32_t k_max_header_len = sizeof(RDTHeader);
constexpr uint32_t k_max_data_len = k_max_eth_len - k_max_header_len;

struct RDTPacket {
    RDTHeader header;
    uint8_t* data;
    // std::array<uint8_t, k_max_data_len> data;

    RDTPacket(uint8_t type, uint16_t length, uint32_t seq_num, uint8_t* data);
    RDTPacket();
    ~RDTPacket();
    
    void calculate_checksum();

    bool verify_packet() {
        return false;
    }
};


class RDTSocket : public UnreliableSocket {
public:
    RDTSocket(uint32_t window_size);
    virtual ~RDTSocket();

    void accept();
    void connect(const std::string& ip, uint16_t port); 
    void send(uint8_t* data, uint32_t len);
    std::tuple<uint8_t*, size_t> recv();

private:
    // various helper methods
    bool can_recv();
    std::optional<std::unique_ptr<RDTPacket>> recv_packet();
    std::unique_ptr<RDTPacket> recv_packet_block();
    void send_packet(RDTPacket packet);
    void send_and_wait_for_ack(RDTPacket packet);

    uint32_t clamp_seq(uint32_t seq);
public:
    using clock = std::chrono::high_resolution_clock;
    constexpr static auto k_timeout = 500ms;
private:
    uint32_t _window_size;
    struct sockaddr_in _connection;
    socklen_t _conn_len;
    uint32_t _seq;
};