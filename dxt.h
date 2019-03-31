#pragma once
#include <vector>
#include <stdexcept>
#include <array>
#include <cassert>
#include <iostream>
#include "color.h"

inline std::array<Color, 16> DXT1_DecompressBlock(uint64_t data) {
    std::array<Color, 16> res;

    uint8_t* bytes = (uint8_t*)&data;

    Color c1 = Color_Unpack565(((bytes[1] << 8) + bytes[0]));
    Color c2 = Color_Unpack565(((bytes[3] << 8) + bytes[2]));

    Color code2;
    Color code3;
    if(c1.c > c2.c) {
        code2 = Color_2_third(c1, c2);
        code3 = Color_2_third(c2, c1);
    } else {
        code2 = Color_AVG(c1, c2);
        code3 = {0};
    }

    for(int x=0; x<4; x++) {
        for(int y=0; y<4; y++) {
            uint8_t code = (bytes[y+4] >> (x)*2) & 0x03; 
            assert(code < 4);
            switch(code) {
                case 0: res[y*4 + x] = c1; break;
                case 1: res[y*4 + x] = c2; break;
                case 2: res[y*4 + x] = code2; break;
                case 3: res[y*4 + x] = code3; break;
            }
        }
    }

    return res;
}

inline void DXT1_DecompressImage(size_t w, size_t h, const std::vector<uint8_t>& buf, std::vector<uint32_t>& out) {
    if(buf.size() != w*h/2) throw std::invalid_argument("invalid dxt1 size");
    out.resize(w*h);
    size_t offset = 0;
    for(size_t y= 0; y<h; y+=4) {
        for(size_t x=0; x<w; x+=4) {
            uint64_t b = ((uint64_t*)buf.data())[offset++];
            auto block = DXT1_DecompressBlock(b);
            out[(y+0)*w + x] = block[0].c;
            out[(y+0)*w + x + 1] = block[1].c;
            out[(y+0)*w + x + 2] = block[2].c;
            out[(y+0)*w + x + 3] = block[3].c;

            out[(y+1)*w + x] = block[4].c;
            out[(y+1)*w + x + 1] = block[5].c;
            out[(y+1)*w + x + 2] = block[6].c;
            out[(y+1)*w + x + 3] = block[7].c;

            out[(y+2)*w + x] = block[8].c;
            out[(y+2)*w + x + 1] = block[9].c;
            out[(y+2)*w + x + 2] = block[10].c;
            out[(y+2)*w + x + 3] = block[11].c;

            out[(y+3)*w + x] = block[12].c;
            out[(y+3)*w + x + 1] = block[13].c;
            out[(y+3)*w + x + 2] = block[14].c;
            out[(y+3)*w + x + 3] = block[15].c;
        }
    }
}

inline std::array<Color, 16> DXT3_DecompressBlock(uint64_t data, uint64_t data2) {
    std::array<Color, 16> res;
    uint8_t* bytes = (uint8_t*)&data2;
    Color c1 = Color_Unpack565(((bytes[1] << 8) + bytes[0]));
    Color c2 = Color_Unpack565(((bytes[3] << 8) + bytes[2]));
    Color code2 = Color_2_third(c1, c2);
    Color code3 = Color_2_third(c2, c1);
    for(int x=0; x<4; x++) {
        for(int y=0; y<4; y++) {
            uint8_t code = (bytes[y+4] >> (x)*2) & 0x03; 
            assert(code < 4);
            switch(code) {
                case 0: res[y*4 + x] = c1; break;
                case 1: res[y*4 + x] = c2; break;
                case 2: res[y*4 + x] = code2; break;
                case 3: res[y*4 + x] = code3; break;
            }
        }
    }

    uint16_t* shorts = (uint16_t*)&data;
    for(int x=0; x<4; x++) {
        for(int y=0; y<4; y++) {
            uint8_t code = (shorts[3-y] >> (3-x)*4) & 0x0f;
            //res[y*4 + x].argb.a = code << 4;
        }
    }
    /*for(int y=0; y<4; y++) {  
        for(int x=0; x<4; x++) {
            uint8_t code = (x > 1)?bytes[y*2 + 1] : bytes[y*2];
            if(x%2 == 0) res[y*4 + x].argb.a = code & 0xf0;
            else res[y*4 + x].argb.a = (code & 0x0f) << 4;
        }
    }*/

    return res;
}

inline void DXT3_DecompressImage(size_t w, size_t h, const std::vector<uint8_t>& buf, std::vector<uint32_t>& out) {
    if(buf.size() != w*h) throw std::invalid_argument("invalid dxt3 size");
    out.resize(w*h);
    size_t offset = 0;
    for(size_t y= 0; y<h; y+=4) {
        for(size_t x=0; x<w; x+=4) {
            uint64_t b1 = ((uint64_t*)buf.data())[offset++];
            uint64_t b2 = ((uint64_t*)buf.data())[offset++];
            auto block = DXT3_DecompressBlock(b1, b2);
            out[(y+0)*w + x] = block[0].c;
            out[(y+0)*w + x + 1] = block[1].c;
            out[(y+0)*w + x + 2] = block[2].c;
            out[(y+0)*w + x + 3] = block[3].c;

            out[(y+1)*w + x] = block[4].c;
            out[(y+1)*w + x + 1] = block[5].c;
            out[(y+1)*w + x + 2] = block[6].c;
            out[(y+1)*w + x + 3] = block[7].c;

            out[(y+2)*w + x] = block[8].c;
            out[(y+2)*w + x + 1] = block[9].c;
            out[(y+2)*w + x + 2] = block[10].c;
            out[(y+2)*w + x + 3] = block[11].c;

            out[(y+3)*w + x] = block[12].c;
            out[(y+3)*w + x + 1] = block[13].c;
            out[(y+3)*w + x + 2] = block[14].c;
            out[(y+3)*w + x + 3] = block[15].c;
        }
    }
}