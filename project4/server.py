#!/usr/bin/env python3

import socket
import select
import threading
import sys
import re

class Server:
    
    def __init__(self, port: int):
        self.port = port
        self.incoming_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.incoming_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.incoming_sock.bind(("localhost", self.port))

        self.buf_size = 1024 * 8
    
    def get_host(self, status:str) -> str:
        thing = status.split(" ")[1]

    def modify_status_req(self, status: str) -> str:
        pass

    def run(self):
        # listen for incoming requests
        self.incoming_sock.listen(8)

        while True:
            # accept connection
            conn, addr = self.incoming_sock.accept() 

            readable, _, _ = select.select([conn], [], [], 2.0)
            
            # if not readable after 2 seconds, close connection and move on
            if len(readable) < 1:
                print(f"INFO: No request after {2.0} seconds")
                conn.send("HTTP1/1 500 Internal Server Error\r\n\r\n".encode())
                conn.close()
                continue
         
            req = conn.recv(self.buf_size).decode()
            print(f"INFO: Request Received:\n {req}")
            
            # split message into parts
            splits = req.split("\r\n")

            # match = re.match(r"GET http://((?:\w+\.)+\w+)(?::(\d+))?(/(?:\w+/)*) HTTP/1\.1", splits[0])
            # match = re.match(r"GET http://((?:\w+\.)+\w+)(?::(\d+))?(/.*) HTTP/1\.1", splits[0])
            match = re.match(r"GET http://((?:\w[\w\-]*\.)+\w[\w\-]*)(?::(\d+))?(/.*) HTTP/1\.1", splits[0])

            if match is None:
                print("Invalid header")
                conn.close()
                continue

            groups = match.groups()

            domain_str = groups[0]
            port_str = groups[1]
            path_str = groups[2]

            print(f"domain: {domain_str}, port: {port_str}, path: {path_str}")
        
            port = 80
            if port_str is not None and not port_str == "":
                port = int(port_str)

            # connect to server
            out_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            status = out_socket.connect_ex((domain_str, port))
            if status != 0:
                # TODO: return error frame
                no_connect_err = "HTTP/1.1 502 Bad Gateway\r\n\r\n"
                conn.send(no_connect_err.encode())
                out_socket.close()
                conn.close()
                continue

            req_out = f"GET {path_str} HTTP/1.1\r\n"
            for i in range(1, len(splits)):
                if re.match(r"Connection: keep-alive", splits[i]) is not None:
                    continue 
                req_out += splits[i]
                req_out += "\r\n"
            req_out += "Connection: Close\r\n"
            req_out += "\r\n"

            print(f"req out: {req_out}")

            out_socket.send(req_out.encode())

            readable, _, _ = select.select([out_socket], [], [], 10.0)
            
            # if no response from server, close connection and throw an error           
            if len(readable) < 1:
                print(f"Connection timed out")
                out_socket.close()
                
                # TODO: send error message back to client
                timeout_err = "HTTP/1.1 504 Gateway Timeout\r\n\r\n"
                conn.send(timeout_err.encode())

                # close connection to client
                conn.close()
                # continue
                continue
            
            while True:
                res = out_socket.recv(self.buf_size)

                if len(res) == 0:
                    break 

                conn.send(res)

            conn.close()

def main():
    if len(sys.argv) < 2:
        print("Usage: proxy <port>")
        exit(1)

    port = int(sys.argv[1])
    server = Server(port)
    server.run()

if __name__ == "__main__":
    main()
