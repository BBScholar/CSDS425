#!/usr/bin/env python3

import sys

from rdt import RDTSocket

def main():
    server_name = sys.argv[1]
    server_port = int(sys.argv[2])
    window_size = int(sys.argv[3])
    filename = sys.argv[4]

    socket = RDTSocket(window_size)

    # wait until we are connected
    socket.connect(server_name, server_port)
    
    with open(filename) as f:
        data = f.read()
        socket.send(data.encode())

    socket.close()


if __name__ == "__main__":
    main()
