import socket
import select
import time
import random

class UnreliableSocket:

    def __init__(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        self.temp_packet= None

        self.to_reorder = False
        self.reorder_next = False
        self.duplicate_count = 0

    def bind(self, addr: str, port: int):
        self.sock.bind((addr, port))
        print(f"Bound to {addr}:{port}")

    def recvfrom(self, bufsize=4096):

        if self.to_reorder and self.reorder_next and self.temp_packet is not None:
            print("reordering")
            ret = self.temp_packet
            self.temp_packet = None
            self.to_reorder = False
            self.reorder_next = False
            return ret

        if self.duplicate_count > 0 and self.temp_packet is not None:
            print("generating duplicates")
            self.duplicate_count -= 1
            ret = self.temp_packet
            if self.duplicate_count == 0:
                self.temp_packet = None
            return ret


        start_time = time.time()
        while time.time() - start_time < 0.1:
            readable, _, _ = select.select([self.sock], [], [], 0)

            if len(readable) == 0:
                continue

            self.sock.setblocking(False)
            data, addr = self.sock.recvfrom(bufsize)
            self.sock.setblocking(True)

            if len(data) == 0:
                time.sleep(0.001)
                continue

            # drop packet
            if random.random() < 0.1:
                continue
            elif random.random() < 0.1:
                bad_idx = random.randint(0, len(data) - 1)
                arr = bytearray(data)
                arr[bad_idx] = 0
                data = bytes(arr)
            elif random.random() < 0.1:
                self.temp_packet = (addr, data)
                self.to_reorder = True
                self.reorder_next = False
                continue
            elif random.random() < 0.1:
                self.duplicate_count = random.randint(2, 4)
                self.duplicate_count -= 1
                self.temp_packet = (addr, data)

            if self.to_reorder and not self.reorder_next:
                self.reorder_next = True

            return addr, data

        return None, None

    def sendto(self, addr, data: bytes):
        self.sock.sendto(data, addr)

    def close(self):
        self.sock.close()

    
