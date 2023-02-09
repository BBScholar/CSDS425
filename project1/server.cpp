#include <cstdio>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <cinttypes>
#include <csignal>
#include <cstdlib>
#include <cstring>

#include <unordered_set>
#include <vector>
#include <array>
#include <sstream>
#include <unordered_map>
#include <memory>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

#include "common.h"

// this is real ugly
// not sure if there's a better alternative
struct Client {
    struct sockaddr* addr;
    socklen_t len;

    Client(socklen_t plen, struct sockaddr* paddr) {
        len = plen;
        addr = static_cast<struct sockaddr*>(std::malloc(len));
        std::memcpy(addr, paddr, plen);
    }

    Client(Client& other) = delete;

    ~Client() {
        std::free(addr);
    }
};

int main(int argc, char** argv) {

    argc--;
    argv++;

    int socket_fd;
    
    struct sockaddr_in server;

    if(argc < 1) {
        std::cout << "Not enough arguments. Usage: server <port>" << std::endl;
        return 1;
    }
    
    unsigned short port;
    try {
        port =  (unsigned short) std::stoul(argv[0]);
    } catch(const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return 2;
    }
    std::memset(&server, 0, sizeof(server));
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::perror("socket");
        return 2;
    }

    if((bind(socket_fd, (struct sockaddr*)&server, sizeof(server))) < 0) {
        std::perror("bind");
        return 3;
    }

    std::cout << "Server Initialized..." << std::endl;

    // allocate buffers
    struct sockaddr new_client;
    socklen_t len;

    std::array<char, k_buf_len> recv_buf, send_buf, host_buf, service_buf;

    std::unordered_map<std::string, std::unique_ptr<Client>> clients;

    while(1) {
        std::memset(&new_client, 0, sizeof(new_client));
        std::size_t n = recvfrom(socket_fd, recv_buf.data(), k_buf_len - 1, 0, &new_client, &len);

        int s = getnameinfo((struct sockaddr*)&new_client, sizeof(new_client), host_buf.data(), NI_MAXHOST,
                   service_buf.data(), NI_MAXSERV, NI_NUMERICSERV);
    
        if(s == 0) {
            std::cout << "Received " << n << " bytes from " << host_buf.data() << ":" << service_buf.data() << std::endl;
        } else {
            std::cerr << "getnameinfo: " << gai_strerror(s) << std::endl; 
            continue;
        }

        const std::string host_str(host_buf.data());
        const std::string service_str(service_buf.data());
        
        std::string key("");
        key += host_str;
        key += ":";
        key += service_str;

        const auto packet_type = (uint8_t) recv_buf[0];
        
        // janky workaround to prevent double-free
        auto c = std::make_unique<Client>(len, &new_client);

        if(packet_type == PacketGreeting) {

            if(clients.contains(key)) {
                // TODO: duplicate addr 
                std::cout << "Client " << key << " already in list." << std::endl;
                continue;
            }

            std::cout << "Adding client to list: " << key << std::endl;

            clients.insert({key, std::move(c)});
        } else if(packet_type == PacketMessage) {

            send_buf[0] = PacketType::PacketIncoming;
            // TODO: how do I encode the data?
            std::sprintf(&send_buf[1], "%s\0%s\0%s", host_str, service_str, message);

            for(auto iter = clients.begin(); iter != clients.end(); ++iter) {
                if(sendto(socket_fd, msg.data(), msg.size() + 1, 0, iter->second->addr, iter->second->len) < 0) {
                    // TODO: error here
                    perror("send to all clients");
                    continue;
                }
            }
            
        } else {
            std::cout << "Packet with type " << packet_type << " was not processed." << std::endl;
            continue;
        }
    }

    close(socket_fd);

    return 0;
}
