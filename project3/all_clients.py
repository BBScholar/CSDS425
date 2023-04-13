#!/usr/bin/env python3

import sys

from threading import Thread

from client import Client 

# function to run a single instance within a thread
def run(addr: str, port: int, node: str):
    client = Client(addr, port, node)
    client.run()

# main
def main():
    args = sys.argv

    if len(args) < 2:
        print("Usage: all_clients <addr> <port>")
        exit(1)

    addr = args[1]
    port = int(args[2])

    nodes = ["u", "v", "w", "x", "y", "z"]
    threads = []
    for node in nodes:
        t = Thread(target=run, args=[addr, port, node]) 
        t.start()
        threads.append(t)

    for t in threads:
        t.join()

if __name__ == "__main__":
    main()
