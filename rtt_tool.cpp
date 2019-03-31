#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include "dxt.h"
#include "util.h"
#include "png.h"
#include "options.h"

size_t calc_mipmap_size(size_t width, size_t height, int levels) {
    if(levels == 1) return width*height;
    else return width*height + calc_mipmap_size(width/2, height/2, levels - 1);
}

int analyse(std::string filename, size_t maxflen, bool small, bool exp, bool no_mipmaps, const std::string& outdir) {
    std::ifstream file(filename, std::ios::binary);
    auto size = std::experimental::filesystem::file_size(filename);

    std::string printfile = filename;
    if(printfile.size() > maxflen - 1) printfile = printfile.substr(0, maxflen - 1);
    auto filllen = maxflen - printfile.size();

    if(small) std::cout << printfile<< std::string(filllen, ' ');

    auto header_filesize = read32(file);
    // First byte needs to be 0x80 | Maybe header size ?
    if(((header_filesize >> 24) & 0xff) != 0x80) {
        std::cerr << "First byte is not 0x80!" << std::endl;
        return -2;
    }
    header_filesize = header_filesize & 0xffffff;
    if(small) std::cout << "size_expect=" << header_filesize << " ";
    else std::cout << "Expected size: " << header_filesize << std::endl;
    if(header_filesize != (size_t)size - 4) {
        std::cerr << "Size missmatch " << header_filesize << " != " << ((size_t)size-4) << std::endl;
        return -2;
    }

    if(small) std::cout << "data_size=" << header_filesize - 124 << " ";
    else std::cout << "Data size: " << header_filesize - 124 << std::endl;
    uint8_t format = read8(file); // Maybe format ? (6 = 0.5 byte, 1/7/8 = 1byte, 5 = 4byte)
    uint8_t unknown2 = read8(file);
    uint8_t unknown3 = read8(file);
    uint8_t unknown4 = read8(file);
    uint16_t img_width = read16(file);
    uint16_t img_height = read16(file);
    uint8_t unknown5 = read8(file);
    uint8_t unknown6 = read8(file);
    uint8_t mipmap_cnt = read8(file); // Number of Mipmap levels ?
    uint8_t unknown8 = read8(file); // Somehow influences byte/pixel ratio, maybe some sort of mipmaping again ? multiplies by ~54,9

    size_t num_pixels = calc_mipmap_size(img_width, img_height, mipmap_cnt);

    if(small) std::cout << (int)format << " " << (int)unknown2 << " " << (int)unknown3 << " " << (int)unknown4 <<  " width="<<img_width << " height=" << img_height << " " << (int)unknown5 << " " << (int)unknown6 << " mipmaps=" << (int)mipmap_cnt << " " << (int)unknown8 << "\t\tdata_factor=" << ((double)(header_filesize-124) / num_pixels);
    else {
        std::cout << (int)format << " " << (int)unknown2 << " " << (int)unknown3 << " " << (int)unknown4 << std::endl;
        std::cout << "width="<<img_width<<std::endl;
        std::cout << "height=" << img_height << std::endl;
        std::cout << (int)unknown5 << " " << (int)unknown6 << std::endl;
        std::cout << "mipmaps=" << (int)mipmap_cnt << std::endl;
        std::cout << (int)unknown8 << std::endl;
        std::cout << "data_factor=" << ((double)(header_filesize-124) / num_pixels);
    }
    std::cout << std::endl;
    for(int i=0; i< 28; i++) {
        uint32_t x = read32(file);
        if(x) {
            std::cerr << "Nullsegment is not null: " << x << " " << file.tellg() << std::endl;
            //return -2;
        }
    }

    if(exp) {
        if(outdir != "") {
            std::experimental::filesystem::path p(outdir + filename);
            p = p.parent_path();
            if(!std::experimental::filesystem::exists(p)) {
                std::experimental::filesystem::create_directories(p);
            }
        }

        size_t size = img_width*img_height;
        // Try to export data
        for(size_t m =0; m<mipmap_cnt; m++) {
            if(img_height /(1 <<m) < 4 || img_width /(1 <<m) < 4) {
                std::cout << "Ignoring mipmaps with size smaller then 4 pixels" << std::endl;
                break;
            }
            if(format == 6) {
                std::vector<uint8_t> buf(size/2);
                file.read((char*)buf.data(), buf.size());
                std::vector<uint32_t> outbuf;
                DXT1_DecompressImage(img_width/(1 <<m), img_height/(1 <<m), buf, outbuf);
                IMG_invert_vertical(img_width/(1 <<m), img_height/(1 <<m), outbuf);
                PNG_write(img_width/(1 <<m), img_height/(1 <<m), (const uint8_t*)outbuf.data(), outdir + filename + "_"+std::to_string(m) + ".png");
            } else if(format == 7 || format == 8) {
                // DXT3 or DXT5
                std::vector<uint8_t> buf(size);
                file.read((char*)buf.data(), buf.size());
                std::vector<uint32_t> outbuf;
                DXT3_DecompressImage(img_width/(1 <<m), img_height/(1 <<m), buf, outbuf);
                IMG_invert_vertical(img_width/(1 <<m), img_height/(1 <<m), outbuf);
                PNG_write(img_width/(1 <<m), img_height/(1 <<m), (const uint8_t*)outbuf.data(), outdir + filename + "_"+std::to_string(m) + ".png");
            } else {
                std::ofstream out(outdir + filename + "_"+std::to_string(m) + ".data");
                for(size_t i=0; i<size;) {
                    if(format == 5) {
                        uint8_t byte = read8(file);
                        out.write((const char*)&byte, 1);
                        byte = read8(file);
                        out.write((const char*)&byte, 1);
                        byte = read8(file);
                        out.write((const char*)&byte, 1);
                        byte = read8(file);
                        out.write((const char*)&byte, 1);
                        i+=1;
                    } else {
                        uint8_t byte = read8(file);
                        out.write((const char*)&byte, 1);
                        i+=1;
                    }
                }
            }
            size/=4;
            if(no_mipmaps) break;
        }
    }
    return 0;
}

const static std::vector<options::option> opts = {
    { false, true, false, "small", "s" },
    { false, true, false, "export", "e" },
    { false, true, false, "no-mipmaps", "nm" },
    { false, true, false, "continue", "c" },
    { false, false, true, "outdir", "o" }
};

int main(int argc, const char** argv) {
    const auto args = options::parse(opts, argc, argv);

    if(args.extra_args.empty()) {
        std::cerr << argv[0] << " <filename>" <<std::endl;
        return -1;
    }

    std::string outdir = args.has_opt("outdir") ? args.options.at("outdir")[0] : "";

    size_t maxflen = 0;
    for(auto& file: args.extra_args) {
        if(file.size() > maxflen) maxflen = file.size();
    }

    for(auto& file : args.extra_args) {
        if(analyse(file, maxflen + 2, args.has_opt("small"), args.has_opt("export"), args.has_opt("no-mipmaps"), outdir)) {
            if(!args.has_opt("continue")) {
                std::cerr << "Failed, exiting" << std::endl;
                return -2;
            } else {
                std::cerr << "Failed to process " << file << std::endl;
            }
        }
    }
}   