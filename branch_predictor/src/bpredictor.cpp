
#include "bpredictor_defines.h"



//-------------------------------------------------Main Function--------------------------------------------------
int main(int argc, char* argv[]){
    int             m=std::stoi(argv[1]), n=argc > 2 ? std::stoi(argv[2]) : 0;           //m -> low order PC bit to form prediction table index with 2^m entries;  n -> 
    char            outcome;
    unsigned long   pc;
    unsigned        misprediction=0;
    unsigned        total_instr=0;
    unsigned long   index;
    bTable BHT(m,n);
    states _outcome;
    

    while(true){
        std::cin >> std::hex >> pc >> outcome;
        _outcome = (outcome == 't' ) ? states::TAKEN : states::NOT_TAKEN;
        if(pc==-1){
            break;
        }
        BHT.total_instr ++;
        unsigned long tag  = (pc >> 2)&((1<<m)-1);  //Ignoring 2 LSB bit - word aligned instruction
        
        if(n==0){
            index= tag & ( (1 << m)-1 );
        }
        else{
            index= ( (tag & ((1 << (m-n))-1)) | ((BHT.BHR^(tag>>(m-n))) << (m-n)) ) ;
        }
        if( BHT.predict(index, outcome) != _outcome ){
            BHT.misprediction++;
        }
    }
    BHT.print_bTable();
    return 0;   
}