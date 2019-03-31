#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include <libpng/png.h>

inline void PNG_write(size_t img_w, size_t img_h, const uint8_t* data, const std::string& filename) {
    FILE* fp = fopen(filename.c_str(), "wb");
    if(!fp) throw std::runtime_error("Could not open file");
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fclose(fp);
        throw std::runtime_error("Failed to create write struct");
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        fclose(fp);
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        throw std::runtime_error("Failed to create info struct");
    }
    if(setjmp(png_jmpbuf(png_ptr))) {
        fclose(fp);
        png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        throw std::runtime_error("Error during png creation");
    }
    png_byte** row_pointers = (png_byte**)png_malloc(png_ptr, img_h * sizeof(png_byte *));
    for (size_t y = 0; y < img_h; ++y) {
        row_pointers[y] = (uint8_t*)&data[y*img_w*4];
    }

    png_init_io (png_ptr, fp);
    png_set_rows (png_ptr, info_ptr, row_pointers);

    png_set_IHDR(png_ptr, info_ptr, img_w, img_h,
            8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    png_free(png_ptr, row_pointers);
    fclose(fp);
    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
}