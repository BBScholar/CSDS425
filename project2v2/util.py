
import zlib

import struct
from dataclasses import dataclass
from enum import IntEnum

header_format = "!IIII"
header_size = struct.calcsize(header_format)
max_data_len = 1472 - header_size
# for testing
# max_data_len = 100 - header_size

print(f"Header size: {header_size}")
print(f"Max data len: {max_data_len}")

class PacketType(IntEnum):
    Start = 0
    End = 1
    Data = 2 
    Ack = 3

@dataclass(frozen=False)
class PacketHeader:
    ptype: int
    seq: int
    checksum: int
    length: int = 0

def compute_crc(ptype: PacketType, length: int, seq: int, data) -> int:
    buffer = bytes()
    buffer += bytes(int(ptype))
    buffer += bytes(length)
    buffer += bytes(seq)
    if data is not None:
        buffer += data

    return zlib.crc32(buffer)


class Packet:

    def __init__(self, ptype: PacketType, length: int, seq: int, data, checksum=None):
        if checksum is None:
            crc = compute_crc(ptype, length, seq, data)
            header = PacketHeader(ptype=ptype, length=length, seq=seq, checksum=crc)
            self.header = header
        else:
            header = PacketHeader(ptype=ptype, length=length, seq=seq, checksum=checksum)
            self.header = header
        self.data = data

    def to_bytes(self) -> bytes:
        data = bytes()
        data += struct.pack(header_format, int(self.header.ptype), self.header.length, self.header.seq, self.header.checksum)
        if self.data is not None:
            data += self.data

        return data

    @staticmethod
    def from_bytes(data: bytes):
        n = len(data)
        data_slice = None
        if n < header_size:
            return None
        elif n > header_size:
            data_slice = data[header_size:]

        ptype, length, seq, checksum = struct.unpack(header_format, data[0:header_size])
        return Packet(ptype=PacketType(ptype), seq=seq, length=length, checksum=checksum, data=data_slice)

    def valid(self) -> bool:
        left = self.header.checksum
        right = compute_crc(PacketType(self.header.ptype), self.header.length, self.header.seq, self.data)
        ret = left == right
        if not ret:
            print(f"Invalid packet: {left} != {right}")
            return False

        if self.data is not None:
            ret = ret and len(self.data) == self.header.length
            if not ret:
                print(f"length mismatch: {len(self.data)} != {self.header.length}")

        return ret

def header_to_bytes(header: PacketHeader) -> bytes:
    return struct.pack(header_format, header.ptype, header.seq, header.length, header.checksum)

def bytes_to_header(data: bytes):
    ptype, seq, length, checksum = struct.unpack(header_format, data)
    return PacketHeader(ptype, seq, length, checksum)
