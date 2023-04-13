#!/usr/bin/env python3

import socket
import select
import threading
import sys

class Server:
    
    def __init__(self, port: int):
        self.port = port
        self.incoming_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.incoming_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.incoming_sock.bind(("127.0.0.1", self.port))

        self.buf_size = 4096
    
    def get_host(self, status:str) -> str:
        thing = status.split(" ")[1]

    def modify_status_req(self, status: str) -> str:
        pass

    def run(self):
        self.incoming_sock.listen(8)

        while True:
            conn, addr = self.incoming_sock.accept() 

            req = conn.recv(self.buf_size).decode()
            splits = req.split("\r\n")
        
            remote_host = self.get_host(splits[0])
            splits[0] = self.modify_status_req(splits[0])

            req_out = ""
            for s in splits:
                req_out += s

            out_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            out_socket.connect("")
            out_socket.send(bytes(req_out))

            res = out_socket.recv(4096 * 2).decode()
            conn.send(res.encode())

def main():
    if len(sys.argv) < 2:
        print("Usage: proxy <port>")
        exit(1)

    port = int(sys.argv[1])
    server = Server()
    server.run()

if __name__ == "__main__":
    main()
