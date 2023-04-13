#!/usr/bin/env python3

from enum import IntEnum
import socket
import select
import zlib

class PacketType(IntEnum):
    Data = 0
    Ack = 1
    Start = 2 
    End = 3

class PacketHeader:
    pass

class Packet:
    pass

class UnreliableSocket:

    def __init__(self):
        pass

class RDTSocket:

    def __init__(self, window_size: int):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.window_size = window_size
        self.seq = 0
        self.conn = None

    def accept(self, port) -> Tuple[]:
        self.sock.bind(("localhost", port))
        while True:
            packet : Packet = self.recv_packet()

            if packet.type == PacketType.Start:
                break

        ack_packet = Packet()

        send_packet()

    def connect(self, addr: str, port: int):
        pass


    def send(self, data: bytes):
        n = len(data)

        packets = []

        # TODO: create packets        
        while True:
            pass
        


    def recv(self) -> bytes:
        window = [None] * self.window_size

        def clamp(x):
            return x % self.window_size

        while True:
            packet = self.recv_packet()


    def close(self):
        self.sock.close()

    def u_recvfrom(self):

        pass


    def recv_packet(self, timeout):
        r, _, _ = select.select([self.sock], None, None, timeout)
        
        if not r:
            return None 
        
        data, addr = self.sock.recvfrom()


