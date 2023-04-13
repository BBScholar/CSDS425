#include <cstdint>
#include <iostream>
#include <fstream>

#include "util.h"
#include "rdt.h"


int main(int argc, char** argv) {
    argc--;
    argv++;

    if(argc < 2) {
        std::cout << "Invalid Usage." << std::endl;
        std::cout << "Usage: receiver <recv port> <window size>" << std::endl;
        return 1;
    }

    const auto port = parse_int<uint16_t>(argv[0]);
    const auto window_size = parse_int<uint32_t>(argv[1]);

    
    RDTSocket socket(window_size);

    socket.accept();
    
    auto [data, len] = socket.recv();

    std::ofstream file("download.txt");
    file.write((char*) data, len);
    file.close();
    return 0;
}
