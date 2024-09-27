#include "cache.h"
using namespace std;
std::string bin_to_hex(std::string addr_bin){
    while(addr_bin.length()%4 != 0){
        addr_bin.append("0");
    }
    int dec=0;
    std::string hex_addr="";
    std::string digit="";
    for(int i=0; i<addr_bin.length()/4; i++){
        for(int j=0; j<4; j++){
            dec+= (addr_bin[4*i+j]-'0') * pow(2,j);
        }
        char digit;
        if(dec>9){
            digit= (dec-10) + 'A';
        }else{
            digit= (dec-0) + '0';
        }
        hex_addr+=(digit);
        dec=0;
    }
    return hex_addr;
}
unsigned int bin_to_dec(std::string bin, int start, int end){
     int i=start; 
     int len=end-start+1;
     unsigned int dec=0;
     while(len--){
        dec += (int)(bin[i]-'0') * pow(2,i-start);
        i++;
     }
     return dec;
}

// Convert Decimal Address to 32bit Binary string in little endian format 
std::string dec_to_bin(unsigned int addr_dec){
    unsigned int n=addr_dec;
    int i=0;
    std::string addr_bin="";
    while(i<32){
        std::string ch=std::to_string(n%2);
        addr_bin.append((std::string)(ch));
        n/=2;
        i++;
    }
    return addr_bin;
}

// Convert a hexadecimal number in string format to unsigned integer
int hex_to_dec(std::string str_addr){    
    unsigned int dec=0;
    int i =0;
    int n = str_addr.length() -1 ;  
    while(str_addr[i] != '\0'){
        if( (str_addr[i] >= 'a' ) )
        {
            dec += (int) (str_addr[i] - 'a' + 10) * pow(16,n--);
        }
        else{
            dec += (int) (str_addr[i] - '0') * pow(16,n--);   
        }
        i++;
    }
    return dec;
}

std::string hex_to_bin(std::string addr){
    unsigned dec= hex_to_dec(addr);
    std::string bin= dec_to_bin(dec);
    return bin;
}