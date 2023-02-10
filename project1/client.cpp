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
#include <functional>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

#include "common.h"

// helper function for sending json data over the socket
void socketsend(int fd, const json& data) {
    // convert json to string
    std::string str = data.dump();
    
    // send data, exit on fail
    // do not send null terminator because datagram packet 
    // already encodes the length of the data
    if(send(fd, str.data(), str.size(), 0) < 0) {
        perror("send");
        exit(3);
    }
}

int main(int argc, char** argv) {
    argc--;
    argv++;
    
    // file descriptor
    int socket_fd;
    
    // check that we have enough arguments to run the client 
    // need argument for server address and server port
    if(argc < 2) {
        std::cerr << "Not enough arguments. Usage: client <server_address> <port>" << std::endl;
        return 1;
    }
    
    // construct this data because it exists
    std::string host_str(argv[0]), port_str(argv[1]);

    // server setup
    struct hostent* hp;
    struct sockaddr_in server_addr;
    
    // this function allows names as well as raw IPs to be used 
    // for the host field. If an IP is input, it will more or less pass
    // straight through this function, else it will determine what the IP is from the input host 
    // Ex: allows for localhost to be used as the host instead of having to input 127.0.0.1
    hp = gethostbyname(host_str.c_str());
    
    // if this fails, we cannot find the host
    if(!hp) {
        fprintf(stderr, "unknown host\n"); 
        return 2;
    }
    
    // we need to convert the input port to an unsigned short 
    // in order for the htons function to be able to use it 
    // we will use std::stoul to convert the string to an unsigned long 
    // then cast that to an unsigned short
    // throw an hunman readable error if port input is invalid
    unsigned short port;
    try {
        port = std::stoul(port_str);
    } catch(const std::invalid_argument& e) {
        std::cerr << "Invalid port number entered" << std::endl;
        return 2;
    }
    
    // construct server address struct for future ability to send data to server 
    // configure as IPV4 protocol
    server_addr.sin_family = AF_INET;
    // specify port we calculated earlier
    server_addr.sin_port = htons(port);
    // copy 
    std::memcpy((char*)&server_addr.sin_addr, hp->h_addr_list[0], hp->h_length); 

    // socket setup
    if((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return 2;
    }
    
    // 'connect' to socket. for dgram this basically just sets the default send/recv server_address
    // for the corresponding syscalls. 
    if(connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        return 3;
    }
    
    std::cout << "\u001B[2J"; // clear screen
    std::cout << "\u001B[999B"; // move cursor all the way down
    std::cout << "Client started..." << std::endl;
    
    // Send greeting message
    // just needs the one field
    // no need to wait for ack because assignment
    {
        json greet = {
            {"type", "GREETING"}
        };

        socketsend(socket_fd, greet);
    }
    std::cout << "] " << std::flush;

    
    // read buffer with way too much data
    std::array<char, k_buf_len> buf;

    // setup select data
    fd_set read_set;

    // setup select timeout
    struct timeval tv;


    // enter main loop
    while(1) {
        // we need to configure the select systemcall in order to poll both file descriptors
        // first we zero the set of files that need to be polled for read operations
        // all data needs to be set on every loop iterator for... reasons
        FD_ZERO(&read_set);
        // add stdin fd to set
        FD_SET(0, &read_set);
        // add socket fd to set
        FD_SET(socket_fd, &read_set);

        // max fd used in select syscall, needs to be equal to max file descriptors plus 1
        // no idea why
        const int max_fd = socket_fd + 1;
        
        // set timeout
        // if no data is recieved on either file descriptor in this time span
        // the select systemcall will return with that state.
        // 5 seconds seems reasonable
        // this needs to be set on every loop because the select call updates this value with remaining delay time
        // as the program runs
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        // wait for data
        int ready = select(max_fd, &read_set, NULL, NULL, &tv);
    

        // check the output value from select systemcall 
        // if -1, there is an error in the systemcall
        // if ready is equal to 0, the select call timed out
        if(ready == -1)  {
            std::cerr << "select()" << std::endl;
            continue; // not sure if should be exiting here or not
        } else if(!ready) {
            continue;
        }
    
        // check if stdin is ready to read 
        // if so, we must handle input
        if(FD_ISSET(0, &read_set)) {

            // get line from stdin
            // we can safely assume that the user has pressed enter 
            // before we get here
            std::string s;
            std::getline(std::cin, s);
    
            // go up a line and delete it
            // this gets rid of the message that was just entered
            // additionally, print our ] prompt again
            std::cout << "\u001B[1A" << "\u001B[2K" << "] " << std::flush;
        
            // check if message is too large
            // don't want to send massive packets plus my buffers have finite size
            if(s.size() > k_max_msg_len) {
                std::cerr << "Message too long, failed to send. Max length is " << k_max_msg_len << " characters" << std::endl; 
                continue;
            } else if(s == "") {
                // if input string is empty, do nothing, don't waste time/bandwidth
                continue;
            }
        
            // create json message object
            // packet type is MESSAGE 
            // copy our message into json packet
            json data = {
                {"type", "MESSAGE"},
                {"message", s}
            };
            
            // send packet
            socketsend(socket_fd, data);
        }

        // check if socket is ready to read 
        // if so, we must handle data
        if(FD_ISSET(socket_fd, &read_set)) {
            // recieve data from socket, make sure we only take
            // as many characters as the buffer can hold minus 1 for the null terminator that must be added
            int n = recv(socket_fd, buf.data(), buf.size() - 1, 0);
            // set null terminator
            buf[n] = '\0';

            // parse json from packet
            // throw error if malformed
            json data;
            try {
                data = json::parse(buf.data());
            } catch(const json::parse_error& e) {
                std::cerr << e.what() << std::endl;
                continue;
            }
            
            // check if the packet has the required type field
            // if not, can't process packet, just continue looping
            if(!data.contains("type")) {
                // does not have correct fields
                continue;
            }
            
            // get packet type now that we know it exists
            const auto packet_type = data["type"].get<std::string>();
            
            // check the packet type. If packet type is not incoming, 
            // we do not care about it.
            if(packet_type != "INCOMING") continue;
            
            // check if incoming packet has required data fields
            if(!data.contains("origin") || !data.contains("message")) continue;
        
            // get host and message data now that we know it exists
            const auto host = data["origin"].get<std::string>();
            const auto message = data["message"].get<std::string>();
        
            //  define color ranges
            //  225 on the 8-bit color table seemed reasonable, 
            //  anything after that is just grey
            constexpr uint8_t min_color = 1;
            constexpr uint8_t max_color = 225;
            constexpr uint8_t color_range = max_color - min_color + 1;
            
            //  hash the incoming origin string
            std::size_t h = std::hash<std::string>{}(host);
            // map hash to color
            uint8_t color = (h % color_range) + min_color;
            
            // Control code reference:
            // https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
            std::cout << "\u001B[s" << "\u001B[A" << "\u001B[999D" << "\u001B[S" << "\u001B[L"; // ansi magic
            std::cout << "\u001B[38;5;" << std::to_string(color) << "m"; // set color
            std::cout << "<From: " << host << ">: "; // print sender info
            std::cout << message << '\n'; // print message
            std::cout << "\u001B[38;5;255m"; // set text color back to white
            std::cout << "\a"; // terminal bell  for the memes
            std::cout << "\u001B[u" << std::flush;
        }
    }

    close(socket_fd);

    return 0;
}
