#!/usr/bin/env python3

import socket 
import select
import json
import sys
import threading

import util


class Server:
    
    def __init__(self, port: int, config_fn: str):
        # create socket
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        # bind to address
        self.sock.bind(("localhost", port))
        # socket starts listening
        self.sock.listen(8)

        # initialize table
        self.table = self.read_table(config_fn)

    def read_table(self, fn: str):
        with open(fn) as f:
            s = f.read()
            table = json.loads(s)

        return table

    def run(self):
        # wait for all clients to join
        while True:
            c_sock, addr = self.sock.accept()
            print(f"New connection from {str(addr)}")

            data = c_sock.recv(4096)
            json_data = json.loads(data.decode()) #TODO: error check
            msg_type = json_data["type"]

            if msg_type == "JOIN":
                node_name = json_data["node_name"]
                
                # check for duplicate entries
                if "socket" in self.table[node_name]:
                    print(f"Duplicate join message from client {node_name}")
                    continue

                # add socket and address to table for layer use 
                self.table[node_name]["socket"] = c_sock
                self.table[node_name]["addr"] = addr

                print(f"Node with name {node_name} comes from {addr}")
            else:
                print("invalid message type")
                continue
            
            # check if every router has connected 
            # if so, we can exit the join state
            exit_join = True
            for k in self.table.keys():
                if "socket" not in self.table[k]:
                    exit_join = False 

            if exit_join:
                break
        
        # send initial data to clients
        for k in self.table.keys():

            join_res_message = {
                "type": "JOIN_RES",
                "table" : self.table[k]["neighbors"]
            }

            print(f"sending data to {k}: {join_res_message}")

            self.table[k]["socket"].send(json.dumps(join_res_message).encode())
        
        # wait for updates
        while True:
            # listen to all sockets
            readable, _, _ = select.select([self.table[k]["socket"] for k in self.table.keys()], [], [])
            
            #check each client for updates
            for s in readable:
                data = s.recv(4096).decode()
                updates = util.split_json(data)

                if len(updates) == 0:
                    continue

                for u in updates:
                    print(f"Received update: {repr(u)}")
                    if u["type"] != "UPDATE":
                        continue

                    # change type for forwarding
                    u["type"] = "UPDATE_RES"
                    #
                    node_name = u["node_name"]
                    # send neighors the packets
                    for n in self.table[node_name]["neighbors"].keys():
                        # skip non-neighbors
                        if self.table[node_name]["neighbors"][n] == -1:
                            continue
                        # send packet
                        print(f"Sending message: {repr(u)}")
                        self.table[n]["socket"].send(json.dumps(u).encode())


def main():
    port = int(sys.argv[1])
    config_fn = sys.argv[2]
    server = Server(port, config_fn)
    server.run()

if __name__ == "__main__":
    main()
