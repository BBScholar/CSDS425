#include "rdt.h"
#include "util.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <limits>
#include <memory>
#include <tuple>
#include <vector>
#include <algorithm>
#include <thread>
#include <random>

#include <sys/select.h>
#include <sys/time.h>

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

// using namespace std::chrono;
using namespace std::chrono_literals;

RDTPacket::RDTPacket(uint8_t type, uint16_t length, uint32_t seq_num, uint8_t* d)  {
    header.type = type;
    header.length = length;
    header.seq_num = seq_num;
    // calculate_checksum(data, length);

    data = new uint8_t[length];
    std::memcpy(data, d, length);
}

RDTPacket::RDTPacket() : data(nullptr) {

}

RDTPacket::~RDTPacket() {
    delete[] data;
}


// RDT socket definitions

RDTSocket::RDTSocket(uint32_t window_size) 
    : _window_size(window_size) {

}

RDTSocket::~RDTSocket() {

}

void RDTPacket::calculate_checksum() {

}

void RDTSocket::accept() {
    // recieve packet, wait until we receive a start packet
    std::optional<std::unique_ptr<RDTPacket>> packet;
    do {
        packet = recv_packet();
    } while(packet.has_value());// TODO
    
    // create ack packet
    RDTPacket ack_pkt;
    ack_pkt.header.type = HeaderType::Ack;
    ack_pkt.header.seq_num = packet.value()->header.seq_num; // TODO: check
    ack_pkt.header.length = 0;
    ack_pkt.data = nullptr;
    ack_pkt.calculate_checksum();
    
    // send ack
    send_packet(ack_pkt);
}

bool RDTSocket::can_recv() {
    fd_set read;

    FD_ZERO(&read);
    FD_SET(_fd, &read);
}

void RDTSocket::send_packet(RDTPacket packet) {
    const auto len = packet.header.length + k_max_header_len;

    std::array<uint8_t, k_max_eth_len> buf;

    std::memcpy(buf.data(), &packet.header, k_max_header_len);
    std::memcpy(&buf.data()[k_max_header_len], packet.data, packet.header.length);

    const int n = k_max_header_len + packet.header.length;

    if(u_sendto(buf.data(), n, (sockaddr*) &_connection, _conn_len) < 0) {
        // some sort of error
    }
}

std::optional<std::unique_ptr<RDTPacket>> RDTSocket::recv_packet() {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(_fd, &read_set);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    const int r = select(_fd + 1, &read_set, nullptr, nullptr, &tv);

    if(!FD_ISSET(_fd, &read_set)) {
        return {};
    }

    std::array<uint8_t, k_max_eth_len> buf;
    struct sockaddr_storage addr;
    socklen_t addr_len;
    const auto n = u_recvfrom(buf.data(), buf.size(), (sockaddr*) &addr, &addr_len);

    // dont accept packet if it is on the wrong protocol somehow
    if(addr.ss_family != AF_INET) {
        return {};
    }

    // check if this packet comes from the correct source
    struct sockaddr_in *addr_in = (sockaddr_in*) &addr;
    if(addr_in->sin_addr.s_addr != _connection.sin_addr.s_addr || addr_in->sin_port != _connection.sin_port) {
        return {};
    }

    // check if packet meets minimum length
    if(n < k_max_data_len) {
        return {};
    }

    // calculate observed data length
    const auto data_len = n - k_max_header_len;

    // create packet object
    auto ret_packet = std::make_unique<RDTPacket>();

    // this is possibly not safe due to struct padding, reordering, or architecture
    // differences
    std::memcpy(&ret_packet->header, buf.data(), k_max_header_len);

    // check if observed vs expected data lengths match
    if(ret_packet->header.length != data_len) {
        return {};
    }

    // otherwise allocate data
    ret_packet->data = new uint8_t[data_len];
    std::memcpy(&ret_packet->data, &buf.data()[k_max_header_len], data_len);

    // check data, return nothing if checksum bad
    if(!ret_packet->verify_packet()) {
        return {};
    }

    return ret_packet;
}

std::unique_ptr<RDTPacket> RDTSocket::recv_packet_block() {
    std::optional<std::unique_ptr<RDTPacket>> packet;
    do {
        packet = recv_packet();
        std::this_thread::sleep_for(1ms);
    } while(!packet.has_value());

    return std::move(packet.value());
}

void RDTSocket::send_and_wait_for_ack(RDTPacket packet) {
    while(true) {
        send_packet(packet);

    }
}

void RDTSocket::connect(const std::string &ip, uint16_t port) {

    struct hostent* hp;
    struct sockaddr_in server_addr;

    hp = gethostbyname(ip.c_str());

    if(!hp) {
        std::cerr << "Could not find host " << ip << std::endl;
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    std::memcpy(&server_addr.sin_addr, hp->h_addr_list[0], hp->h_length);

    _connection = server_addr;
    _conn_len = sizeof(struct sockaddr_in);

    // generate random start seq number
    std::random_device rd;
    std::uniform_int_distribution<uint32_t> rand(0, std::numeric_limits<uint32_t>::max());
    const uint32_t start_seq = rand(rd);
    
    // construct start packet
    RDTPacket start_pkt;
    start_pkt.header.type = HeaderType::Start;
    start_pkt.header.length = 0;
    start_pkt.header.seq_num = start_seq;
    start_pkt.data = nullptr;
    start_pkt.calculate_checksum();

    send_and_wait_for_ack(start_pkt);
    _seq = start_seq;
}

void RDTSocket::send(uint8_t* data, uint32_t len) {
    const uint32_t n = len / k_max_data_len + 1;

    // reserve storage for packets
    std::vector<RDTPacket> packets;
    packets.reserve(n);

    // generate packets
    for(int i = 0; i < n; ++i) {
        uint16_t length, checksum;
        uint32_t seq_num = _seq++;

        length = std::min(k_max_data_len, len);
        len -= length;

        uint8_t* data = &data[k_max_data_len * i];
        packets.emplace_back(RDTPacket(HeaderType::Data, length, seq_num, data));
    }
        
    uint32_t send_base, nextseqnum;
    
    auto last_ack = clock::now();

    while(send_base != n) {
        // send packets in window 
        for(int i = send_base; i < std::min(send_base + _window_size, n); ++i ) {
            send_packet(packets[send_base + i]);
        }
        last_ack = clock::now();
        
        // wait for acks
        while((clock::now() - last_ack) < k_timeout) {
            std::optional<std::unique_ptr<RDTPacket>> packet = recv_packet();

            if(packet.has_value()) {
                // TODO: do something
            }

            // TODO: lots of logic here
            std::this_thread::sleep_for(1ms);
        }

        // this means we timed out or possible duplicate ack
    }
}

std::tuple<uint8_t*, size_t> RDTSocket::recv() {
    // create window
    // std::unique_ptr<RDTPacket[]> window(new RDTPacket[_window_size]);
    std::vector<std::unique_ptr<RDTPacket>> window;

    for(auto& p : window) {
        p.reset(nullptr);
    }
    
    std::vector<std::unique_ptr<RDTPacket>> packets;

    uint32_t N;
    
    while(true) { 
        // recieve packet
        std::unique_ptr<RDTPacket> packet = recv_packet_block();

        // break if end packet
        if(packet->header.type == HeaderType::End) {
            // TODO: send ack?
            break;
        }
        
        // calculate min and max seq numbers we are accepting
        const int min_seq = N;
        const int max_seq = N + _window_size - 1;
        
        // if we are within the window, do window related activities
        if(packet->header.seq_num >= min_seq && packet->header.seq_num <= max_seq) { 
            // copy new packet into circ buf
            window[clamp_seq(packet->header.seq_num)] = std::move(packet);

            while (window[clamp_seq(N)]->header.type != HeaderType::None) {
                // copy packet out
                // packets.push_back(std::unique_ptr(nullptr));
                packets.push_back(std::move(window[clamp_seq(N)]));

                // set packet type to none
                window[clamp_seq(N)]->header.type = HeaderType::None;

                // increment N
                ++N;
            }
        } 

        // send cummulative ack 
        RDTPacket ack_packet;
        ack_packet.header.type = HeaderType::Ack;
        ack_packet.header.seq_num = N;
        ack_packet.header.length = 0;
        ack_packet.data = nullptr;
        ack_packet.calculate_checksum();
        send_packet(ack_packet);
    }
    
    // done receiving packets
    const auto n = k_max_eth_len * packets.size();
    uint8_t* data = new uint8_t[n];

    size_t counter = 0;

    for(const auto& p : packets) {
        // TODO: copy data from each packet
        std::memcpy(data, &p->data[counter], p->header.length);
        counter += p->header.length;
    }
     
    return {data, n};
}

uint32_t RDTSocket::clamp_seq(uint32_t seq) {
    return seq % _window_size;
}