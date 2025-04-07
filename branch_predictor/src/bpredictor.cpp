
#include "bpredictor_defines.h"



//-------------------------------------------------Main Function--------------------------------------------------
int main(){
    int             m, n=N;           //m -> low order PC bit to form prediction table index with 2^m entries;  n -> 
    char            outcome;
    unsigned long   pc;
    unsigned        misprediction=0;
    unsigned        total_instr=0;
    bTable BHT;
    states _outcome;
    

    while(true){
        std::cin >> std::hex >> pc >> outcome;
        _outcome = (outcome == 't' ) ? states::TAKEN : states::NOT_TAKEN;
        if(pc==-1){
            break;
        }
        BHT.total_instr ++;
        unsigned long tag  = (pc >> 2)&((1<<M)-1);  //Ignoring 2 LSB bit - word aligned instruction
        
        if(n==0){
            unsigned long index= tag & ( (1 << M)-1 );
            if( BHT.predict_bimodal(index, outcome) != _outcome ){
                BHT.misprediction++;
            }
        }
        else{
            unsigned long index= ( (tag & ((1 << (M-N))-1)) | ((BHT.BHR^(tag>>(M-N))) << (M-N)) ) ;
            if( BHT.predict_gShare(index, outcome) != _outcome ){
                BHT.misprediction++;
            }
        }
    }
    BHT.print_bTable();
    return 0;
}