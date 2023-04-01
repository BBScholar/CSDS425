#!/usr/bin/env python3

import sys 

from rdt import RDTSocket

def main():
    if len(sys.argv) < 4:
        print("Usage: receiver <port> <window_size> <filename>")
        exit(1)

    port = int(sys.argv[1])
    window_size = int(sys.argv[2])
    filename = sys.argv[3]

    socket = RDTSocket(window_size)
    socket.bind("localhost", port)
    socket.accept()

    data = socket.receive()

    with open(filename, "w") as f:
        f.write(data.decode())
 
if __name__ == "__main__":
    main()
