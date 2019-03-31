#pragma once
#include <cstdint>

inline uint32_t swap_bytes(uint32_t x) {
    uint32_t res = (x & 0xff);
    res = res << 8 | (x >> 8 & 0xff);
    res = res << 8 | (x >> 16 & 0xff);
    res = res << 8 | (x >> 24 & 0xff);
    return res;
}

inline uint16_t swap_bytes(uint16_t x) {
    uint16_t res = (x & 0xff);
    res = res << 8 | (x >> 8 & 0xff);
    return res;
}

uint32_t read32(std::ifstream& file) {
    uint32_t x;
    file.read((char*)&x, sizeof(x));
    return swap_bytes(x);
}

uint16_t read16(std::ifstream& file) {
    uint16_t x;
    file.read((char*)&x, sizeof(x));
    return swap_bytes(x);
}

uint8_t read8(std::ifstream& file) {
    uint8_t x;
    file.read((char*)&x, sizeof(x));
    return x;
}

void IMG_invert_vertical(size_t img_w, size_t img_h, std::vector<uint32_t>& data) {
    for(size_t i=0; i< img_h/2; i++) {
        for(size_t x = 0; x<img_w; x++) {
            auto tmp = data[i*img_w + x];
            data[i*img_w + x] = data[(img_h - i - 1)*img_w + x];
            data[(img_h - i - 1)*img_w + x] = tmp;
        }
    }
}