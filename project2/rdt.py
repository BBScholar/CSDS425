from usocket import UnreliableSocket
import util
from util import PacketHeader, PacketType, Packet
import math
import random
import select
import time

class RDTSocket(UnreliableSocket):

    def __init__(self, window_size):
        super().__init__()
        self.window_size = window_size
        self.current_connection = None
        self.timeout = 0.5
        self.seq = 0

    def connect(self, addr, port):
        self.current_connection = (addr, port)
        self.seq = random.randint(0, 1000)

        start_packet = Packet(ptype=PacketType.Start, seq=self.seq, length=0, data=None)

        while True:
            self.sendto_packet(start_packet, (addr, port))
            done = False
            start_time = time.time()
            while time.time() - start_time < 0.5:
                recv_packet, recv_addr = self.recvfrom_packet()

                # cont if no packet
                if recv_packet is None:
                    continue
                # cont if not from current addr
                # elif recv_addr != (addr, port):
                #     print("incorrect addr")
                #     print(f"{recv_addr} != {(addr, port)}")
                #     continue
                # cont if not ack packet
                elif recv_packet.header.ptype != PacketType.Ack or recv_packet.header.seq != self.seq:
                    print("Wrong packet type or bad seq")
                    continue

                done = True
                break

            if done:
                break

        self.current_connection = (addr, port)

    def accept(self):
        while True:
            recv_packet, addr = self.recvfrom_packet()
            if recv_packet is None:
                continue
            elif not recv_packet.valid() or recv_packet.header.ptype != PacketType.Start:
                print(f"Packet valid: {recv_packet.valid()}")
                continue

            self.seq = recv_packet.header.seq
            self.current_connection = addr

            # send ack
            ack_packet = Packet(ptype=PacketType.Ack, seq=self.seq, length=0, data=None)

            self.sendto_packet(ack_packet, self.current_connection)
            break



    def send(self, data: bytes):
        num_packets= int(math.ceil(float(len(data)) / float(util.max_data_len)))
        max_seq = self.seq + num_packets - 1

        n = len(data)

        packets = []

        # generate packets
        for i in range(num_packets):

            length = min(util.max_data_len, n)
            n -= length

            j = i * util.max_data_len
            data_slice = data[j:j+length]

            p = Packet(ptype=PacketType.Data, seq=self.seq + i, length=length, data=data_slice)

            packets.append(p)

        starting_seq = self.seq

        def to_idx(x: int) -> int:
            return x - starting_seq

        done = False
        while self.seq <= max_seq:
            # send packets
            min_send = to_idx(self.seq)
            max_send = min(to_idx(self.seq + self.window_size), len(packets))
            for i in range(min_send, max_send):
                self.sendto_packet(packets[i], self.current_connection)

            # wait for response
            start_wait = time.time()
            while time.time() - start_wait < self.timeout:
                packet, addr = self.recvfrom_packet()
                if packet is None:
                    continue
                # elif addr != self.current_connection:
                #     continue
                elif packet.header.ptype != PacketType.Ack:
                    continue

                if packet.header.seq == max_seq:
                    done = True
                    self.seq = packet.header.seq
                    break

                if self.seq <= packet.header.seq:
                    diff = packet.header.seq - self.seq
                    self.seq = packet.header.seq + 1

                    new_max_send = min(max_send + diff, len(packets))
                    # send new packets
                    for i in range(max_send, new_max_send):
                        self.sendto_packet(packets[i], self.current_connection)

                    max_send = new_max_send

                    # reset timer
                    start_wait = time.time()

            if done:
                break

            print("Timer timed out, retransmitting...")

        # done sending data
        end_packet = Packet(ptype=PacketType.End, length=0, seq=self.seq, data=None)

        while True:
            self.sendto_packet(end_packet, self.current_connection)

            start_wait = time.time()
            done = False
            while time.time() - start_wait < 0.5:
                packet, addr = self.recvfrom_packet()

                if packet is None:
                    continue
                elif packet.header.ptype != PacketType.Ack or packet.header.seq != self.seq:
                    continue

                done = True
            if done:
                break

        print("Done sending")



    def receive(self) -> bytes:
        window = [None] * self.window_size
        recved_packets = []

        while True:
            min_seq = self.seq
            max_seq = self.seq + self.window_size - 1

            packet, recv_addr = self.recvfrom_packet()

            if packet is None:
                # print("packet is none")
                continue
            elif packet.header.ptype != PacketType.End and packet.header.ptype != PacketType.Data:
                print("packet has wrong type")
                continue
            elif not packet.valid():
                print("packet not valid")
                if packet.data is not None:
                    pass
                    #print(packet.data)
                continue

            if packet.header.ptype == PacketType.End:
                print(f"{window}")
                end_ack = Packet(ptype=PacketType.Ack, length=0, seq=packet.header.seq, data=None)
                self.sendto_packet(end_ack, self.current_connection)
                break

            recv_seq = packet.header.seq
            if recv_seq < self.seq or recv_seq > max_seq:
                print("seq out of range")
                ack_packet = Packet(seq=self.seq - 1, length=0, ptype=PacketType.Ack, data=None)
                self.sendto_packet(ack_packet, self.current_connection)
                continue

            # store packet if its within the window
            #if window[self.clamp_seq(recv_seq)] is None:
            window[self.clamp_seq(recv_seq)] = packet
            # print(window)

            # move window as needed
            while window[self.clamp_seq(self.seq)] is not None:
                recved_packets.append(window[self.clamp_seq(self.seq)])
                window[self.clamp_seq(self.seq)] = None
                self.seq += 1

            # send ack packet
            ack_packet = Packet(seq=self.seq - 1, length=0, ptype=PacketType.Ack, data=None)
            self.sendto_packet(ack_packet, self.current_connection)

        self.current_connection = None
        out = bytes()
        for p in recved_packets:
            # print(p)
            out += p.data
        return out


    def sendto_packet(self, packet: Packet, addr):
        print(f"Sending packet with seq={packet.header.seq} of type {packet.header.ptype}")
        super().sendto(addr, packet.to_bytes())

    def recvfrom_packet(self):
        addr, data= super().recvfrom(4096)

        if data is not None:
            # print(f"from {addr} got data: {data}")
            pass
        else:
            return None, None

        packet = Packet.from_bytes(data)
        if packet is None:
            # print("Could not convert to packet")
            return None, None
        # print(f"Received {packet.header.ptype} packet with seq={packet.header.seq}")
        return packet, addr

    def clamp_seq(self, x: int) -> int:
        return x % self.window_size



