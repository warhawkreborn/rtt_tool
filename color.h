#pragma once
#include <cstdint>

typedef union {
    uint32_t c;    // packed representation
    struct {
        uint8_t b;    // (reversed for intel storage order)
        uint8_t g;
        uint8_t r;
        uint8_t a;
    } argb;
} Color;

inline Color Color_BLACK() {
    Color res = {0};
    res.argb.a = 255;
    return res;
}

inline Color Color_Unpack565(uint16_t v) {
    Color res;
    res.argb.a = 255;
    res.argb.r = (v & 0xF800) >> 8;
    res.argb.g = (v & 0x07E0) >> 3;
    res.argb.b = (v & 0x1F) << 3;
    return res;
}

inline Color Color_AVG(Color c1, Color c2) {
    Color res;
    int t = c1.argb.r + c2.argb.r;
    res.argb.r = t/2;
    t = c1.argb.g + c2.argb.g;
    res.argb.g = t/2;
    t = c1.argb.b + c2.argb.b;
    res.argb.b = t/2;
    t = c1.argb.a + c2.argb.a;
    res.argb.a = t/2;
    return res;
}

inline Color Color_2_third(Color c1, Color c2) {
    Color res;
    int t =c1.argb.r + c1.argb.r + c2.argb.r;
    res.argb.r = t/3;
    t = c1.argb.g + c1.argb.g + c2.argb.g;
    res.argb.g = t/3;
    t = c1.argb.b + c1.argb.b + c2.argb.b;
    res.argb.b = t/3;
    t = c1.argb.a + c1.argb.a + c2.argb.a;
    res.argb.a = t/3;
    return res;
}

inline Color Color_FromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    Color res;
    res.argb.r = r;
    res.argb.g = g;
    res.argb.b = b;
    res.argb.a = a;
    return res;
}
