#ifndef __CACHE_DEPENDENCIES__
#define __CACHE_DEPENDENCIES__
#include <string.h>
#include<math.h>
#include <string.h>
#include<fstream>
#include<stdlib.h>

    unsigned int bin_to_dec(std::string bin, int start, int end);
    std::string dec_to_bin(unsigned int addr_dec);
    int hex_to_dec(std::string str_addr);
    std::string bin_to_hex(std::string addr_bin);
#endif