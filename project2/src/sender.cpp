#include <cstdlib>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include "rdt.h"
#include "util.h"


int main(int argc, char** argv) {
    argc--;
    argv++;
    
    // we need 3 arguments, exit program otherwise
    if(argc < 3) {
        std::cout << "Invalid Usage." << std::endl;
        std::cout << "Usage: sender <recv addr> <recv port> <window size>" << std::endl;
        return 1;
    }
    
    // get server ip
    std::string server_ip = argv[0];
    // parse server port
    const auto server_port = parse_int<uint16_t>(argv[1]);
    // parse window size
    const auto window_size = parse_int<uint32_t>(argv[2]);


    // reverse dns lookup for ip
    // struct hostent* hp;
    //
    // hp = gethostbyname(server_ip.c_str());
    //
    // if(!hp) {
    //     std::cerr << "Unknown host" << std::endl;
    //     return 2;
    // }

    // open file to send
    std::ifstream file("alice.txt");

    if(!file.is_open()) {
        std::cerr << "Could not open alice.txt" << std::endl;
        return 3;
    }
    
    std::stringstream ss;
    ss << file.rdbuf(); 
    file.close();

    auto str = ss.str();
    uint8_t* data = reinterpret_cast<uint8_t *>(str.data());
    
    RDTSocket socket(window_size);
    socket.connect(server_ip, server_port);
    socket.send(data, str.length());

    return 0;
}
