#!/usr/bin/env python3

from threading import Thread

from client import Client 

def run(node: str):
    client = Client("localhost", 5555, node)
    client.run()


def main():
    nodes = ["u", "v", "w", "x", "y", "z"]
    threads = []
    for node in nodes:
        t = Thread(target=run, args=[node]) 
        t.start()
        threads.append(t)

    for t in threads:
        t.join()

if __name__ == "__main__":
    main()
