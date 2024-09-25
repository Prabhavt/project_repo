#ifndef __CACHE_DEPENDENCIES__
#define __CACHE_DEPENDENCIES__
#include<iostream>
#include<math.h>
#include<string>
#include<fstream>
#include<stdlib.h>
#include<vector>

    unsigned int bin_to_dec(std::string bin, int start, int end);
    std::string dec_to_bin(unsigned int addr_dec);
    int hex_to_dec(std::string str_addr);
    std::string bin_to_hex(std::string addr_bin);
    std::string hex_to_bin(std::string addr);
#endif