#pragma once 


#include <cstddef>
#include <cstdint>
#include <array>

constexpr std::array<uint32_t, 256> make_crc32() {
    constexpr uint32_t POLYNOMIAL = 0xEDB88320;

    std::array<uint32_t, 256> crcTable;
    unsigned long remainder;
    unsigned char b = 0;

    do {
        // Start with the data byte
        remainder = b;
        for (unsigned long bit = 8; bit > 0; --bit) {
            if (remainder & 1)
                remainder = (remainder >> 1) ^ POLYNOMIAL;
            else
                remainder = (remainder >> 1);
        }
        crcTable[(size_t)b] = remainder;
    } while(0 != ++b);

    return crcTable;
}

constexpr std::array<uint32_t, 256> crc32 = make_crc32();