#include "bpredictor_defines.h"
std::string hex2bin_address_32(std::string pc){
    std::string pc_bin; 
    int size_pc = pc.length();                      //size_pc -> number of nonzero digits of PC
    int rem, div;
    for(int i=size_pc - 1; i >= 0; i--){
        int num= (pc[i]-'0') < 10 ? pc[i]-'0' : pc[i]-'a'+10; 
        int itr=4;                                  //To represent every hex digit as 4 bits
        while(itr>0){
            pc_bin.append((num%2)?"1":"0");
            num/=2;
            itr--;
        }  
    }
    std::cout<<std::endl;
    return pc_bin;
} 

int bin2dec(std::string addr){
    int index   =0;
    int bin     =1;
    for(int i=0; i<addr.length(); i++){
        index+= (addr[i]-'0')*bin;
        bin *=2;
    }
    return index;
}