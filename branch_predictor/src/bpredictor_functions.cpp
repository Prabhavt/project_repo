#include "bpredictor_defines.h"

bTableEntry::bTableEntry(int _index, states _pred){
    index =_index;
    pred  = _pred;
}

bTable::bTable(int _m, int _n){
    m=_m;
    n=_n;
    for(int i = 0 ; i < (1<<_m); i++){
        entries.push_back(bTableEntry(i, states::WEAKLY_TAKEN));
    }
    BHR=0;
}

states bTable::predict(int _index, char outcome){
    for( std::list<bTableEntry>::iterator itr= entries.begin(); itr != entries.end(); itr++){
        if( _index == itr -> index ){
            if(itr -> pred <= 1){
                int p            = (outcome == 't') ? (itr->pred+1) : states::NOT_TAKEN;
                BHR              = (outcome == 't') ? (BHR>>1)|(1<<(n-1))&((1<<n)-1) : (BHR >> 1)&((1<<n)-1) ;
                states prev_state= itr->pred;
                itr->pred        = states(p);   
                return prev_state <=1 ? states::NOT_TAKEN : states::TAKEN;
            }else{
                int p    = (outcome == 'n') ? (itr->pred-1) : states::TAKEN;
                BHR      = (outcome == 'n') ? (BHR >> 1)&((1<<n)-1) : (BHR>>1)|(1<<(n-1))&((1<<n)-1);
                states prev_state= itr->pred;
                itr->pred= states(p);
                return prev_state <=1 ? states::NOT_TAKEN : states::TAKEN;
            }
        }
    }
    return states::NOT_TAKEN;
}


void bTable::print_bTable(){
    std::cout<<"OUTPUT"<<std::endl;
    std::cout<<"number of predictions: "<<total_instr<<std::endl;
    std::cout<<"Miss Prediction: "<<misprediction<<std::endl;
    std::cout<<"Miss Prediction (%): "<<(float)misprediction*100/total_instr<<"%"<<std::endl;
    if(n==0)std::cout<<std::endl<<"FINAL   BIMODAL CONTENTS"<<std::endl;
    else std::cout<<std::endl<<"FINAL   gShare CONTENTS"<<std::endl;
    for(std::list<bTableEntry>::iterator itr= entries.begin(); itr != entries.end(); itr++){
        std::cout<<itr->index<<"       "<<itr->pred<<std::endl;
    }
}