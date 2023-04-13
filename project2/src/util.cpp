#include "util.h"
#include "rdt.h"

#include <iostream>
#include <exception>
#include <iterator>
#include <stdexcept>
#include <sys/types.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

UnreliableSocket::UnreliableSocket() {
    _fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(_fd < 0) {
        throw std::exception();
    }

}

UnreliableSocket::~UnreliableSocket() {
    close(_fd); 
}

ssize_t UnreliableSocket::u_bind(uint16_t port) {
        // assign server sockaddr address
    struct sockaddr_in server;
    std::memset(&server, 0, sizeof(server));
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    return bind(_fd, (struct sockaddr*) &server, sizeof(server));
}

ssize_t UnreliableSocket::u_sendto(uint8_t *data, int data_len, const struct sockaddr *dest_addr, socklen_t addr_len) {
    return sendto(_fd, data, data_len, 0, dest_addr, addr_len);
}

ssize_t UnreliableSocket::u_recvfrom(uint8_t *data, int data_len, struct sockaddr *src_addr, socklen_t *addrlen) {
    return recvfrom(_fd, data, data_len, 0, src_addr, addrlen);
}


