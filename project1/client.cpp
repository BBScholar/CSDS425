#include <bits/types/struct_timeval.h>
#include <functional>
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

void socketsend(int fd, const json& data) {
    std::array<char, k_buf_len> buf;

    std::string str = data.dump();

    if(send(fd, str.data(), str.size() + 1, 0) < 0) {
        perror("send");
        exit(3);
    }
}

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

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
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
    
    std::cout << "\u001B[2J"; // clear screen
    std::cout << "Client started..." << std::endl;

    std::array<char, k_buf_len> buf;
    
    // setup select data
    fd_set read_set;

    // setup select timeout
    struct timeval tv;
    
    {
        json greet = {
            {"type", "GREETING"}
        };

        socketsend(socket_fd, greet);
    }
    
    std::cout << "] " << std::flush;

    // enter main loop
    while(1) {
        FD_ZERO(&read_set);
        // add stdin
        FD_SET(0, &read_set);
        // add socket
        FD_SET(socket_fd, &read_set);
        // max fd used in select syscall
        const int max_fd = socket_fd + 1;
        
        // set timeout
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        // wait for data
        int ready = select(max_fd, &read_set, NULL, NULL, &tv);

        if(ready == -1)  {
            std::cerr << "select()" << std::endl;
            continue;
        } else if(!ready) {
            continue;
        }

        if(FD_ISSET(0, &read_set)) {
            std::string s;
            std::getline(std::cin, s);

            std::cout << "\u001B[1A" << "\u001B[2K" << std::flush;

            if(s.size() > k_max_msg_len) {
                std::cerr << "Message too long, failed to send. Max length is " << k_max_msg_len << " characters" << std::endl; 
                continue;
            } else if(s == "") {
                continue;
            }


            json data = {
                {"type", "MESSAGE"},
                {"message", s}
            };
            
            socketsend(socket_fd, data);

            std::cout << "] " << std::flush;
        }
        if(FD_ISSET(socket_fd, &read_set)) {
            int n = recv(socket_fd, buf.data(), buf.size() - 1, 0);
            buf[buf.size() - 1] = '\0';

            json data = json::parse(buf.data());
            const auto packet_type = data["type"].get<std::string>();
            const auto host = data["origin"].get<std::string>();
            const auto message = data["message"].get<std::string>();

            /*
            print("\u001B[s", end="")     # Save current cursor position
            print("\u001B[A", end="")     # Move cursor up one line
            print("\u001B[999D", end="")  # Move cursor to beginning of line
            print("\u001B[S", end="")     # Scroll up/pan window down 1 line
            print("\u001B[L", end="")     # Insert new line
            print(status_msg, end="")     # Print output status msg
            print("\u001B[u", end="")     # Jump ba/
            */
            constexpr uint8_t min_color = 1;
            constexpr uint8_t max_color = 225;
            constexpr uint8_t color_range = max_color - min_color + 1;

            std::size_t h = std::hash<std::string>{}(host);
            uint8_t color = (h % color_range) + min_color;

            // https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
            std::cout << "\u001B[s" << "\u001B[A" << "\u001B[999D" << "\u001B[S" << "\u001B[L";
            std::cout << "\u001B[38;5;" << std::to_string(color) << "m";
            std::cout << "<From: " << host << ">: ";
            std::cout << message << '\n';
            std::cout << "\u001B[38;5;255m";
            std::cout << "\a"; // terminal bell 
            std::cout << "\u001B[u" << std::flush;
        }
    }

    close(socket_fd);

    return 0;
}
