
#include "bpredictor_defines.h"



//-------------------------------------------------Main Function--------------------------------------------------
int main(){
    int             m, n;           //m -> low order PC bit to form prediction table index with 2^m entries;  n -> 
    std::string     pc;
    char            outcome;
    std::string     filepath= "/home/prabhav/LocalDisk/project_repo/branch_predictor/testbench/sample_trace.txt";
    std::ifstream   file(filepath);
    std::string     line;

    if(!file.is_open()){
        std::cerr << "Error Opening File" << std::endl;
        return -1;
    }

    while(std::getline(file, line)){
        std::istringstream inp_stream(line);
        inp_stream >> pc >> outcome;
        
        //std::cout<<pc<<" ";
        //std::cout<<hex2bin_address_32(pc)<<" \n";
    }

    std::string test="110100111000";
    std::cout<<bin2dec(test);
    return 0;
}