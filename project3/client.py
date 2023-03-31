#!/usr/bin/env python3 


import socket 
import json
import select

import sys
import time

import util


class Client:

    def __init__(self, server_addr: str, server_port: int, name: str):
        self.name = name
        # create socket
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # connect to server
        # self.sock.connect((server_addr, server_port))
        self.addr = (server_addr, server_port)

        # no table yet
        self.table = None

        self.print_timeout = 0.5

    def run_dv(self):
        new_table = {}

        def conv(x: int) -> float:
            if x == -1:
                return float("inf")
            return float(x)

        for y in self.all_nodes:
            # new_table[y] = {}
            min_dist = float("inf")
            min_node = None

            for n in self.neighbors:
                dist = conv(self.table[self.name][n]["dist"])

                if n != y:
                    dist += conv(self.table[n][y]["dist"])

                if dist < min_dist:
                    min_dist = dist
                    min_node = [n]
                    if n != y:
                        min_node.extend(self.table[n][y]["next"])

            if min_dist == float('inf'):
                new_table[y] = {
                    "dist":-1,
                    "next":[]
                }
            else:
                new_table[y] = {
                    "dist": min_dist,
                    "next": min_node
                }

        return new_table

    def run(self):
        self.sock.connect(self.addr)
        
        # send join message
        join_message = {
            "type" : "JOIN",
            "node_name" : self.name
        }
        self.sock.send(json.dumps(join_message).encode())
        
        #  wait for join response
        j_data = {}
        while True:
            data = self.sock.recv(4096)
            j_data = json.loads(data.decode())

            if j_data["type"] == "JOIN_RES":
                break

        j_data = j_data["table"]

        # neighbors 
        self.all_nodes = [k for k in j_data.keys()]
        self.neighbors = [k for k in j_data.keys() if j_data[k] != -1]

        # create table
        self.table = {}
        # assign out data to the table
        self.table[self.name] = {}
        
        # create table for current node
        for k in j_data.keys():
            self.table[self.name][k] = {}
            self.table[self.name][k]["dist"] = j_data[k]
            # if we are neighbors, set next to that node
            self.table[self.name][k]["next"] = [k] if j_data[k] != -1 else []
    
        for n in self.neighbors:
            self.table[n] = {}
            for k in j_data.keys():
                if n == k:
                    continue
                self.table[n][k] = {}
                self.table[n][k]["dist"] = -1
                self.table[n][k]["next"] = []
    
        first = True

        # start running
        while True:
            # run DV the first time with no extra input
            new_table = self.run_dv()
            
            # if the tables are not equal, send an update message to server
            if first or new_table != self.table[self.name]:
                first = False
                self.table[self.name] = new_table
                # send new table to server
                update_msg = {
                    "type" : "UPDATE",
                    "node_name" : self.name,
                    "table" : new_table
                }
                print(f"Sending message: {repr(update_msg)}")
                self.sock.send(json.dumps(update_msg).encode())

            # time.sleep(0.05)

            actual_updates = []
            while True:
                updates = util.split_json(self.sock.recv(4096).decode())

                if len(updates) == 0:
                    continue

                for u in updates:
                    if "type" in u and u["type"] == "UPDATE_RES":
                        actual_updates.append(u)

                if len(actual_updates) > 0:
                    break

            for u in actual_updates:
                # print(f"Received update: {repr(u)}")
                self.table[u["node_name"]] = u["table"]

            # print(f"{self.name}: {repr(self.table)}\n")
            print(f"{self.name}: {json.dumps(self.table, indent=4, sort_keys=True)}\n")

def main():
    server_addr = sys.argv[1]
    server_port = int(sys.argv[2])
    node_name = sys.argv[3]

    client = Client(server_addr, server_port, node_name)

    client.run()


if __name__ == "__main__":
    main()

                
