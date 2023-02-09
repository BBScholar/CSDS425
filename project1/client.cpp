#include <bits/types/struct_timeval.h>
#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <fstream>
#include <cinttypes>
#include <csignal>
#include <cstdlib>
#include <cstring>


#include <sstream>
#include <memory>
#include <array>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

#include "common.h"

int main(int argc, char** argv) {
    argc--;
    argv++;
    
    int socket_fd;
    
    if(argc < 2) {
        std::cerr << "Not enough arguments. Usage: client <server_address> <port>" << std::endl;
        return 1;
    }

    std::string host_str(argv[0]), port_str(argv[1]);

    // server setup
    struct hostent* hp;
    struct sockaddr_in server_addr;
    
    hp = gethostbyname(host_str.c_str());
    
    if(!hp) {
        fprintf(stderr, "unknown host\n"); 
        return 2;
    }
    
    unsigned short port;
    try {
        port = std::stoul(port_str);
    } catch(const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return 2;
    }

    // TODO: check for int conversion error
    //
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    // server_addr.sin_addr.s_addr
    std::memcpy((char*)&server_addr.sin_addr, hp->h_addr_list[0], hp->h_length); 


    // socket setup
    if((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return 2;
    }
    
    // 'connect' to socket. for dgram this basically just sets the default send/recv server_address
    // for the corresponding syscalls
    if(connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        return 3;
    }

    std::cout << "Client started..." << std::endl;
    
    // setup select data
    fd_set read_set;

    // setup select timeout
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    
    // setup read buffer
    std::array<char, k_buf_len> buf;

    // send greeting
    std::fill(std::begin(buf), std::end(buf), 0);
    buf[0] = PacketType::PacketGreeting;

    if(sendto(socket_fd, buf.data(), 1, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("send greeting");
        return 3;
    }
    printf("Greeting sent\n");

    // enter main loop
    while(1) {
        FD_ZERO(&read_set);
        // add stdin
        FD_SET(0, &read_set);
        // add socket
        FD_SET(socket_fd, &read_set);
        // max fd used in select syscall
        const int max_fd = socket_fd + 1;

        // wait for data
        int ready = select(max_fd, &read_set, NULL, NULL, &tv);

        if(ready == -1)  {
            std::cerr << "select()" << std::endl;
            continue;
        } else if(!ready) {
            std::cout << "No data within timeout" << std::endl;
            continue;
        }

        if(FD_ISSET(0, &read_set)) {
            std::cout << "update on command line" << std::endl;
            std::string s;
            std::getline(std::cin, s);

            if(s.size() > k_max_msg_len) {
                std::cerr << "Message too long, failed to send. Max length is " << k_max_msg_len << " characters" << std::endl; 
                continue;
            } else if(s == "") {
                continue;
            }

            buf[0] = PacketType::PacketMessage;
            std::strcpy(&buf[1], s.c_str()); // maybe this should be strncpy
            if(send(socket_fd, buf.data(), 2 + s.size(), 0) < 0) {
                perror("send");
            }
        }
        if(FD_ISSET(socket_fd, &read_set)) {
            std::cout << "update on socket" << std::endl;
            int n = recv(socket_fd, buf.data(), buf.size() - 1, 0);
            buf[buf.size() - 1] = '\0';

            const auto packet_type = (uint8_t) buf[0];
            if(packet_type != PacketType::PacketIncoming) {
                continue;
            }
            const auto addr_len = (uint8_t) buf[1];


            std::string sender_addr, sender_port, message;
            
            std::cout << "<From: " << sender_addr << ":" << sender_port << ">: ";
            std::cout << message << std::endl;
        }
    }

    close(socket_fd);

    return 0;
}
