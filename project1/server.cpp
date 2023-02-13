#include <cstdio>
#include <stdexcept>
#include <cinttypes>
#include <csignal>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <fstream>
#include <array>
#include <unordered_map>
#include <memory>
#include <string>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

#include "common.h"


// struct that holds client data
struct Client {
    struct sockaddr_storage addr;
    socklen_t len;

    Client(struct sockaddr* paddr, socklen_t plen) {
        len = plen;
        std::memcpy(&addr, paddr, plen);
    }

    ~Client() = default;
};


int main(int argc, char** argv) {
    argc--;
    argv++;

    int socket_fd;
    struct sockaddr_in server;
    unsigned short port;
    

    // check if we have an adequite number of arguments
    if(argc < 1) {
        std::cout << "Not enough arguments. Usage: server <port>" << std::endl;
        return 1;
    }
    
    // convert port to unsigned short, make sure to
    // catch errors if no integer was input
    try {
        port =  (unsigned short) std::stoul(argv[0]);
    } catch(const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return 2;
    }
    
    // assign server sockaddr address
    std::memset(&server, 0, sizeof(server));
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    
    // create socket 
    if((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::perror("socket");
        return 2;
    }
    
    // bind to socket
    if((bind(socket_fd, (struct sockaddr*)&server, sizeof(server))) < 0) {
        std::perror("bind");
        return 3;
    }
    
    // print
    std::cout << "Server Initialized..." << std::endl;

    // allocate buffers
    struct sockaddr_storage new_client;
    socklen_t len;
    
    // initialize data
    std::array<char, k_buf_len> recv_buf, send_buf, host_buf, service_buf;
    std::unordered_map<std::string, std::unique_ptr<Client>> clients;

    while(1) {
        // reset parameters
        len = sizeof(new_client);
        std::memset(&new_client, 0, sizeof(new_client));

        // wait for data
        std::size_t n = recvfrom(socket_fd, recv_buf.data(), k_buf_len - 1, 0,
                (struct sockaddr*) &new_client, &len);
        
        // add nul terminator
        recv_buf[n] = '\0';
        
        //lookup name
        int s = getnameinfo((struct sockaddr*)&new_client, len, host_buf.data(), NI_MAXHOST,
                   service_buf.data(), NI_MAXSERV, NI_NUMERICSERV);
    
        // 
        if(s == 0) {
            std::cout << "Received " << n << " bytes from " << host_buf.data() << ":" << service_buf.data() << std::endl;
        } else {
            std::cerr << "getnameinfo: " << gai_strerror(s) << std::endl; 
            continue;
        }
        
        // host and service strings
        const std::string host_str(host_buf.data());
        const std::string service_str(service_buf.data());
        
        // generate key
        std::string key("");
        key += host_str;
        key += ":";
        key += service_str;

        json data;
        
        try {
            data = json::parse(recv_buf);
        } catch (json::parse_error& e) {
            std::cout << "Malformed packet received, ignoring" << std::endl;
            continue;
        }

        // check if the packet has a type
        if(!data.contains("type")) {
                std::cerr << "Invalid packet format" << std::endl;
                continue;
        }
        
        // get packet type
        const auto packet_type = data["type"].get<std::string>();
    
        std::cout << "Packet type: " << packet_type << std::endl;
        
        // janky workaround to prevent double-free
        if(packet_type == "GREETING" ) {
            auto c = std::make_unique<Client>((struct sockaddr*) &new_client, len);
        
            // add client to hashmap
            if(clients.contains(key)) {
                // TODO: duplicate addr 
                std::cout << "Client " << key << " already in list." << std::endl;
                continue;
            }

            std::cout << "Adding client to list: " << key << std::endl;

            clients.insert({key, std::move(c)});
        } else if(packet_type == "MESSAGE" ) {
            if(!data.contains("message")) {
                std::cerr << "No message field found. Skipping. " << std::endl; 
                continue;
            }

            const auto incoming_message = data["message"].get<std::string>();
            
            // create json packet
            json send_packet = {
                {"type", "INCOMING"},
                {"origin", key},
                {"message", incoming_message}
            };
            const std::string msg = send_packet.dump();
            
            // send new message to all clients
            for(auto iter = clients.begin(); iter != clients.end(); ++iter) {
                if(sendto(socket_fd, msg.data(), msg.size(), 0, (struct sockaddr*) &iter->second->addr, iter->second->len) < 0) {
                    perror("send to all clients");
                    continue;
                }
            }
            
        } else {
            // catch error
            std::cout << "Packet with type " << packet_type << " was not processed." << std::endl;
            continue;
        }
    }
    
    // close socket
    close(socket_fd);

    return 0;
}
