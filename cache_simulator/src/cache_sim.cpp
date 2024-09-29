// Author          :  Prabhav Talukdar
// Roll No         :  EE23M053
// Cache Simulator :  Reconfigurable L1+VC+L2
#include"./cache.h"
#include"../testbench/parse.h"
//-------------------------------------------Address Structure----------------------------------------------------------------------
//                      Bit:                 <---log2(CacheSize/Assoc)--->         
//                              31________________________________________0
//                              |  Tag Bits |   Set No.    | Block Offset |
//                              --------------------------------------------   
//-----------------------------------------------------------------------------------------------------------------------------------

//--------------------------------------------Cache Class----------------------------------------------------------------------
class L1{
    std::string         **__cache_tag__;                         
    std::string         **__cachefulladdr__;
    std::string         *__VC__;
    int                 **__cache_valid__;
    int                 **__cache_dirty__;
    int                 *__VC_dirty__;
    int                 **__cache_LRU_count__;
    int                 *__VC_LRU_count__;
    unsigned int        __cache_setno__;
    unsigned int        __cache_read_miss__;
    unsigned int        __cache_write_miss__;
    unsigned int        __cache_reads__;
    unsigned int        __cache_writes__;

    unsigned int        cache, assoc, blocksize, tag, level, victimCache_blocks, num_cache_level;
    std::string              addr_bin;
    std::string              addr_hex;                        
    std::string              writeback_address;
    char                op;
    unsigned int        rd_nextlevel;           // L1 -> L2  or  L2 -> Memory
    unsigned int        wb_nextlevel;     //signals
    unsigned int        write_back;        //count of total write backs from L1
    unsigned int        swaps;
    unsigned int        swap_request;
public:
//--------------------------Initializing Cache information----------------------------------------------------------------------------------------
    L1(int C, int A, int B, int T, int L, int V, int num_level){
        level               =L;
        cache               =C;
    if(C){
        assoc               =A;
        blocksize           =B;
        tag                 =T;
        victimCache_blocks  =V;
        num_cache_level     =num_level;

        __cache_tag__                 = new std::string *[(cache/(assoc*blocksize))];
        __cachefulladdr__   = new std::string *[(cache/(assoc*blocksize))];
        __cache_valid__               = new int    *[(cache/(assoc*blocksize))];
        __cache_dirty__               = new int    *[(cache/(assoc*blocksize))];
        __cache_LRU_count__           = new int    *[(cache/(assoc*blocksize))];
        
        //Assign Victim Cache if level = 1
        swaps           = 0;
        if(L==1 && V !=0){
            __VC__          = new std::string [V];
            __VC_LRU_count__      = new int [V];
            __VC_dirty__    = new int [V];
            swap_request    = 0;
            for(int i=0; i<V; i++){
                __VC_LRU_count__[i]   =V-1;
                __VC__[i]       ="-1";
                __VC_dirty__[i] =0;
            }
        }

        for(int i=0; i<((cache/(assoc*blocksize)));i++){
            __cache_tag__ [i]          = new std::string[assoc];
            __cachefulladdr__[i]= new std::string[assoc];
            __cache_valid__[i]        = new int[assoc];
            __cache_dirty__[i]        = new int[assoc];
            __cache_LRU_count__[i]    = new int[assoc];
        }

        for(int i=0;i<(cache/(assoc*blocksize));i++){
            for(int j=0; j<assoc;j++){
            __cache_tag__ [i][j]       ="-1";
            __cache_valid__[i][j]     =0;              //Valid=0 : Not Valid          Valid=1 : Valid
            __cache_LRU_count__[i][j] =assoc-1;
            __cache_dirty__[i][j]     =0;              //Dirty=0 : Not Modified       Dirty=1 : Modified
            }
        }

        __cache_read_miss__        =0;
        __cache_reads__            =0;
        __cache_writes__           =0;
        __cache_write_miss__       =0;
        rd_nextlevel               =0;
        wb_nextlevel               =0;   
        write_back                 =0;                  
        }
    }
    void put_addr(char operation, std::string address){
        addr_hex            =address;
        op                  =operation;
        unsigned int add_dec=hex_to_dec(address);
        addr_bin            =dec_to_bin(add_dec);
        unsigned int __cache_setno__ =bin_to_dec( addr_bin, ((int)log2(blocksize)) , ((int)log2(cache/assoc))-1  );
        rd_nextlevel             =0; 
        wb_nextlevel       =0;
    //---------------------------Read Operation------------------------------------------------------------------------------------------------------

            if(op=='r'){
                
                //Increment number of Reads 
                __cache_reads__++;

                //Check if any Invalid block is present in Cache(L1/L2) set
                int empty_block =0;
                for(int i=0; i<assoc; i++){
                    if(__cache_valid__[__cache_setno__][i]==0){
                        empty_block=1;
                        break;
                    }                            
                }
                
                
                for(int i=0; i<assoc; i++){
                    std::string mem_FA_tag  =hex_to_bin(__cachefulladdr__[__cache_setno__][i]).substr((int)log2(blocksize),32-(int)log2(blocksize));
                    std::string addr_FA_tag =hex_to_bin(address).substr((int)log2(blocksize),32-(int)log2(blocksize));
                    
                    //----------------------Read Hit-----------------------------------------------
                    //Comparing (tag+index) bits of Requested Address and Cache block address
                    if( !addr_FA_tag.compare(mem_FA_tag) ){
                        //-------LRU Update------------
                        LRU_update(__cache_setno__, i); 
                        break; 
                    }
            
                    //------------------Read Miss-------------------
                    if(i==(assoc-1)){
                        
                        //Incrementing L1 Read miss count
                        __cache_read_miss__+=1;

                        //---------------Read Miss but Cache set have invalid Cache Line--------------------------- 
                        if(empty_block){
                            //-----------------------------------Cold Miss------------------------------- 
                            //---------------------Bring Data to Cold Blocks from Next Level--------------
                            //------------------------Read Signal to next Level--------------------------- 

                            //Signaling Next level cache/memory for Read
                            rd_nextlevel=1;
                            //Reseting empty block flag
                            empty_block=0;

                            for(int j=0; j<assoc; j++){
                                if(__cache_valid__[__cache_setno__][j]==0){
                                    __cache_tag__ [__cache_setno__][j]       =addr_bin.substr((int)log2(cache/assoc),tag);
                                    __cachefulladdr__[__cache_setno__][j]    =address;

                                    __cache_valid__[__cache_setno__][j]                =1;
                                    __cache_dirty__[__cache_setno__][j]                =0;
                                    //-------LRU Update------------
                                    LRU_update(__cache_setno__, j); 
                                    break;
                                }
                            }
                            break;
                        }
                        //---------------Read Miss but all Valid blocks in cache set---------------------------   
                        else if(victimCache_blocks && ~empty_block)
                        {
                            //-------------------If Victim Cache is present----------------------------------------
                            //---------------If L1 cache with Victim Cache present---------------------------------
                            //Increment Swap request
                            swap_request++;
                            int lru_pos         =-1;
                            //-------LRU position in cache------
                            for(int k=0; k<assoc; k++){
                                if(__cache_LRU_count__[__cache_setno__][k]==assoc-1){
                                    lru_pos             =k;
                                    break;
                                }
                            }

                            bool victim_hit = false;
                            int victim_pos  =-1;
                            for(int k=0; k<victimCache_blocks; k++){
                                std::string vc_tag                    =hex_to_bin(__VC__[k]).substr((int)log2(blocksize),32-(int)log2(blocksize));
                                std::string addr_fullyassociative_tag =hex_to_bin(address).substr((int)log2(blocksize),32-(int)log2(blocksize));
                                //Comparing (tag+index) of L1 and tag of VC
                                if(!addr_fullyassociative_tag.compare(vc_tag)){
                                    victim_hit=true;
                                    victim_pos=k;
                                    break;
                                }
                            }
                                
                            //--------Victim Cache Hit-----------
                            if(victim_hit){
                                unsigned int victim_hex_dec =hex_to_dec(__VC__[victim_pos]);
                                std::string victim_bin           =dec_to_bin(victim_hex_dec);
                                
                                __cache_tag__ [__cache_setno__][lru_pos]        =victim_bin.substr((int)log2(cache/assoc),tag);
                                __VC__[victim_pos].swap(__cachefulladdr__[__cache_setno__][lru_pos]);
                                swap(__cache_dirty__[__cache_setno__][lru_pos],__VC_dirty__[victim_pos]);
                                swaps++;

                                LRU_VC_update(victim_pos);
                                //-------LRU Update------------
                                LRU_update(__cache_setno__, lru_pos); 
                            }
                            //--------Victim Cache Miss------------
                            else{
                                //----Read from next level---------
                                //Signaling Next level cache/memory for Read 
                                rd_nextlevel=1;
                                bool invalid_victim_block=false;
                                int victimpos=-1;
                                for(int k=0; k<victimCache_blocks; k++){
                                    if(__VC__[k]=="-1"){
                                        invalid_victim_block=true;
                                        victimpos=k;
                                        break;
                                    }
                                }
                                if(invalid_victim_block){
                                    __VC__[victimpos]                =__cachefulladdr__[__cache_setno__][lru_pos];
                                    __VC_dirty__[victimpos]          = __cache_dirty__[__cache_setno__][lru_pos];
                                    
                                    //Bring in new data to cache
                                    __cache_tag__ [__cache_setno__][lru_pos]              = addr_bin.substr((int)log2(cache/assoc),tag);
                                    __cachefulladdr__[__cache_setno__][lru_pos]= address; 
                                    __cache_dirty__[__cache_setno__][lru_pos]            = 0;   
                                    __cache_valid__[__cache_setno__][lru_pos]            = 1;
                                    
                                    //-------LRU Update------------
                                    LRU_VC_update(victimpos);
                                    LRU_update(__cache_setno__, lru_pos); 
                                }
                                //------Victim Cache Set Full------------
                                else{
                                    for(int k=0; k< victimCache_blocks; k++){
                                        if(__VC_LRU_count__[k]==(victimCache_blocks-1)){
                                            //------Dirty Block has to be written back---------
                                            if(__VC_dirty__[k]){
                                                write_back++;
                                                wb_nextlevel=1;
                                                writeback_address =__VC__[k];
                                            }
                                            __VC__[k]                                 = __cachefulladdr__[__cache_setno__][lru_pos];
                                            __VC_dirty__[k]                           = __cache_dirty__[__cache_setno__][lru_pos];
                                            __cache_tag__ [__cache_setno__][lru_pos]                      = addr_bin.substr((int)log2(cache/assoc),tag);
                                            __cachefulladdr__[__cache_setno__][lru_pos]        = address;
                                            __cache_valid__[__cache_setno__][lru_pos]                    = 1;
                                            __cache_dirty__[__cache_setno__][lru_pos]                    = 0;
                                            
                                            LRU_VC_update(k);
                                            //-------LRU Update------------
                                            LRU_update(__cache_setno__, lru_pos); 
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        else{        
                            rd_nextlevel=1;
                            for(int j=0; j<assoc; j++){
                                if( __cache_LRU_count__[__cache_setno__][j] == (assoc-1) ){                         
                                    if(__cache_dirty__[__cache_setno__][j]){
                                        wb_nextlevel                   =1;
                                        write_back++;
                                        writeback_address               = __cachefulladdr__[__cache_setno__][j];                             
                                    }
                                    __cache_tag__ [__cache_setno__][j]                      = addr_bin.substr((int)log2(cache/assoc),tag);
                                    __cachefulladdr__[__cache_setno__][j]        = address;
                                    __cache_valid__[__cache_setno__][j]                    = 1;
                                    __cache_dirty__[__cache_setno__][j]                    = 0;
                                
                                    //-------LRU Update------------
                                    LRU_update(__cache_setno__, j); 
                                    break;
                                }
                            }
                        }
                    }
                }
            }
//-------------------------------------------------------Write Operation------------------------------------------------------------------------------------------------------

            if(op=='w'){

                //Increment number of Writes
                __cache_writes__++;

                //Check if any Invalid block is present in Cache(L1/L2) set 
                int empty_block  =0;
                //---Checking Line capacity---
                for(int i=0; i<assoc; i++){
                    if(__cache_valid__[__cache_setno__][i]==0){
                        empty_block=1;
                        break;
                    }                            
                }

                for(int i=0; i<assoc; i++){

                    std::string mem_FA_tag  =hex_to_bin(__cachefulladdr__[__cache_setno__][i]).substr((int)log2(blocksize),32-(int)log2(blocksize));
                    std::string addr_FA_tag =hex_to_bin(address).substr((int)log2(blocksize),32-(int)log2(blocksize));
                    //----------------------Write Hit-----------------------------------------------
                    //Comparing (tag+index) bits of Requested Address and Cache block address
                    if( !addr_FA_tag.compare(mem_FA_tag) ){
                        //------Dirty Bit Set----------
                        __cache_dirty__[__cache_setno__][i]    = 1;    
                        //-------LRU Update------------
                        LRU_update(__cache_setno__, i); 
                        break;
                }

                //--------------Write Miss-------------------------------------
                if((i==(assoc-1)))
                {
                    //Incrementing L1 Write miss count
                    __cache_write_miss__   +=1;

                    //-----No Write Allocate if Invalid Cache Block Present--------
                    if(empty_block){

                        //Signaling Next level cache/memory for Read
                        rd_nextlevel          =1;
                        //Reseting empty block flag
                        empty_block      =0;

                        for(int j=0; j<assoc; j++){
                            if(__cache_valid__[__cache_setno__][j]==0){
                                __cache_tag__ [__cache_setno__][j]               =addr_bin.substr((int)log2(cache/assoc),tag);
                                __cachefulladdr__[__cache_setno__][j] =address;

                                __cache_valid__[__cache_setno__][j]=1;
                                __cache_dirty__[__cache_setno__][j]=1;                
                                //-------LRU Update------------
                                LRU_update(__cache_setno__, j); 
                                break;
                            }
                        }            
                    }
                    //-------------------If Victim Cache is present--------------------------------
                    //---------------Write Miss but all Valid lines in Cache set---------------------------   
                    else if(victimCache_blocks && ~empty_block)
                    {
                        //Increment Swap request
                        swap_request++;
                        int lru_pos         =-1;
                        //-------LRU position in cache------
                        for(int k=0; k<assoc; k++){
                            if(__cache_LRU_count__[__cache_setno__][k]==(assoc-1)){
                                lru_pos     =k;
                                break;
                            }
                        }
                        
                        //-----If L1 cache with Victim Cache present------
                        bool victim_hit = false;
                        int victim_pos  =-1;
                        
                        for(int k=0; k<victimCache_blocks; k++){
                                std::string vc_tag                    =hex_to_bin(__VC__[k]).substr((int)log2(blocksize),32-(int)log2(blocksize));
                                std::string addr_fullyassociative_tag =hex_to_bin(address).substr((int)log2(blocksize),32-(int)log2(blocksize));
                                //Comparing (tag+index) of L1 and tag of VC
                                if(!addr_fullyassociative_tag.compare(vc_tag)){
                                victim_hit=true;
                                victim_pos=k;
                                break;
                            }
                        }
                                
                        //--------Victim Cache Hit-----------
                        if(victim_hit){
                            unsigned int victim_hex_dec =hex_to_dec(__VC__[victim_pos]);
                            std::string victim_bin           =dec_to_bin(victim_hex_dec);
                            
                            __cache_tag__ [__cache_setno__][lru_pos]        =victim_bin.substr((int)log2(cache/assoc),tag);
                            __VC__[victim_pos].swap(__cachefulladdr__[__cache_setno__][lru_pos]);
                            __VC_dirty__[victim_pos] = __cache_dirty__[__cache_setno__][lru_pos];
                            __cache_dirty__[__cache_setno__][lru_pos]   =1;
                            swaps++;
                            LRU_VC_update(victim_pos);
                            //-------LRU Update------------
                            LRU_update(__cache_setno__, lru_pos); 
                        }
                        //--------Victim Cache Miss------------
                        else{
                            //----Read from next level and Write Allocate---------
                            //Signaling Next level cache/memory for Read 
                            rd_nextlevel         =1;
                            bool invalid_victim_block=false;
                            int victimpos  =-1;
                            for(int k=0; k<victimCache_blocks; k++){
                                if(__VC__[k]=="-1"){
                                    invalid_victim_block=true;
                                    victimpos=k;
                                    break;
                                }
                            }
                            if(invalid_victim_block){
                                __VC__[victimpos]                =__cachefulladdr__[__cache_setno__][lru_pos];
                                __VC_dirty__[victimpos]          = __cache_dirty__[__cache_setno__][lru_pos];

                                //Bring in new data to cache
                                __cache_tag__ [__cache_setno__][lru_pos]              = addr_bin.substr((int)log2(cache/assoc),tag);
                                __cachefulladdr__[__cache_setno__][lru_pos]= address; 
                                __cache_dirty__[__cache_setno__][lru_pos]            = 1;
                                __cache_valid__[__cache_setno__][lru_pos]            = 1;
                               
                                //------LRU Update------
                                LRU_VC_update(victimpos);
                                LRU_update(__cache_setno__, lru_pos); 
                            }
                            //-----------Victim Cache Set Full---------------
                            else{
                                for(int k=0; k< victimCache_blocks; k++){
                                    if(__VC_LRU_count__[k]==(victimCache_blocks-1)){
                                        //------Dirty Block has to be written back---------
                                        if(__VC_dirty__[k]){
                                            write_back++;
                                            wb_nextlevel                         =1;
                                            writeback_address                     =__VC__[k];
                                        }
                                        __VC__[k]                                 = __cachefulladdr__[__cache_setno__][lru_pos];
                                        __VC_dirty__[k]                           = __cache_dirty__[__cache_setno__][lru_pos];
                                        __cache_tag__ [__cache_setno__][lru_pos]                      = addr_bin.substr((int)log2(cache/assoc),tag);
                                        __cachefulladdr__[__cache_setno__][lru_pos]        = address;
                                        __cache_valid__[__cache_setno__][lru_pos]                    = 1;
                                        __cache_dirty__[__cache_setno__][lru_pos]                    = 1;
                                        
                                        //------Victim Cache LRU update----------
                                        LRU_VC_update(k);
                                        //-------LRU Update------------
                                        LRU_update(__cache_setno__, lru_pos); 
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    else{
                        rd_nextlevel         =1;
                        for(int j=0; j<assoc; j++){
                            //Replace The Lease Recently Used Dirty Block
                            if(__cache_LRU_count__[__cache_setno__][j] == assoc-1){
                                if(__cache_dirty__[__cache_setno__][j]){
                                    //Write allocate and Write in L1 CACHE
                                    write_back++;
                                    wb_nextlevel=1;
                                    writeback_address = __cachefulladdr__[__cache_setno__][j];                                                               
                                }

                                __cache_dirty__[__cache_setno__][j]=1; 
                                __cache_tag__ [__cache_setno__][j] = addr_bin.substr((int)log2(cache/assoc),tag);
                                __cachefulladdr__[__cache_setno__][j]=address;

                                //-------LRU Update------------
                                LRU_update(__cache_setno__, j); 
                                break;
                            }                
                        }
                    }
                }
            }
                
        }
    }

void LRU_VC_update(int pos){
    int lru_count= __VC_LRU_count__[pos];
    for(int i=0; i<victimCache_blocks; i++){
        if(__VC_LRU_count__[i] < lru_count)
            __VC_LRU_count__[i]++;
    }
    __VC_LRU_count__[pos]=0;
}

//---------------------Updating Least Recently Used block Counter--------------------------------------
void LRU_update(int set, int pos){
    int write_lru_count=__cache_LRU_count__[set][pos];
    //-------LRU Update------------
    for (int k=0; k<assoc; k++){
        if(__cache_LRU_count__[set][k] < write_lru_count)
        __cache_LRU_count__[set][k] +=1;
    }
    __cache_LRU_count__[set][pos]= 0;
}
//--------------------Signalling next Level of Memory---------------------------------------------------
int getSignal(){
    if(wb_nextlevel && rd_nextlevel){
        return 2;
    }
    else if(rd_nextlevel && !(wb_nextlevel)){
        return 1;
    }
    return 0;
}

std::string L2_write_addr(){
    return writeback_address;
}
//----------------------------Get Cache info functions-------------------------------------------

float get_cache_hitratio(){  return (cache?(1.0-((float)__cache_read_miss__+__cache_write_miss__)/(__cache_reads__+__cache_writes__)):0);   }
float get_VC_hitratio(){ return (!swaps?0:((float)swaps/swap_request)); }
int get_misses(){ return (__cache_read_miss__+__cache_write_miss__); }
int get_writeback(){ return (write_back); }
int get_totalaccess(){ return (__cache_reads__+__cache_writes__); }
int get_swaprequest(){ return swap_request; }
int get_swaps(){ return swaps;}
//--------------------Sorting Blocks wrt LRU to display them in order----------------------------
// Cache
void sort_cache(std::string cacheline[], int D[], int lru_count[]){
    for(int i=0; i< assoc; i++){
        bool swapped=false;
        for(int j=0; j< assoc-1; j++){
            if(lru_count[j] > lru_count[j+1]){
                swap(lru_count[j],lru_count[j+1]);
                cacheline[j+1].swap(cacheline[j]);
                swap(D[j+1],D[j]);
                swapped=true;
            }
        }
        if(swapped==false) break;
    }
}
// Victim Cache
void sort_vc(std::string vc[], int lru_count[]){
    for(int i=0; i< victimCache_blocks; i++){
        bool swapped=false;
        for(int j=0; j< victimCache_blocks-1; j++){
            if(lru_count[j] > lru_count[j+1]){
                swap(lru_count[j],lru_count[j+1]);
                vc[j+1].swap(vc[j]);
                swapped=true;
            }
        }
        if(swapped==false) break;
    }
}

//-------------------------------------Print CACHE Data------------------------------------------
void print_data(){
        std::cout<<std::endl<<std::endl;
        std::cout<<"=====L"<<level<<" content====="<<std::endl;
        for(int i=0;i<(cache/(assoc*blocksize));i++){
            printf("set no. %3d %4.4s",i,"  | ");
            sort_cache(__cache_tag__ [i],__cache_dirty__[i],__cache_LRU_count__[i]);
        for(int j=0; j<assoc;j++){
            if(__cache_tag__ [i][j]!="-1")
            {       
                std::string hex_add=bin_to_hex(__cache_tag__[i][j]);
                if(hex_add[hex_add.length()-1]=='0') hex_add.erase(hex_add.length()-1);
                for(int i=hex_add.length()-1; i>=0; i--){
                    printf("%c",hex_add[i]);
                }
                if(__cache_dirty__[i][j]){
                    std::cout<<" D";
                }else{
                    std::cout<<"  ";
                }
            }
            else{
                std::cout<<__cache_tag__ [i][j];
            }
            printf("%3.1s","|");
        }
        std::cout<<std::endl;
        }

        if(victimCache_blocks){
        std::cout<<std::endl<<"=======VC Data======="<<std::endl;
        std::cout<<"set 0: ";
        sort_vc(__VC__,__VC_LRU_count__);
        for(int i=0; i<victimCache_blocks; i++){
            unsigned int addrdec=hex_to_dec(__VC__[i]);
            std::string addrbin      =dec_to_bin(addrdec);
            std::string revAddr      =bin_to_hex(addrbin.substr((int)log2(blocksize),32-(int)log2(blocksize)));
            for(int j=revAddr.length()-1; j>=0; j--)
            {    std::cout<<revAddr[j];}
            if(__VC_dirty__[i])
                std::cout<<" D ";
            else
                std::cout<<"  ";
        }     
    }
    std::cout<<std::endl<<std::endl;
}
int print_simulation_result(){
    if(level==1){
        printf("a. L1 read: \t\t\t\t%d\n",__cache_reads__);
        printf("b. L1 read miss: \t\t\t%d\n",__cache_read_miss__);
        printf("c. L1 write: \t\t\t\t%d\n",__cache_writes__);
        printf("d. L1 write miss: \t\t\t%d\n",__cache_write_miss__);
        printf("e. Number of swap request: \t\t%d\n",(victimCache_blocks?swap_request:0));
        printf("f. Swap Request rate: \t\t\t%.4f\n",(float)(victimCache_blocks?((float)swap_request/(__cache_reads__+__cache_writes__)):0));
        printf("g. Number of swaps \t\t\t%d\n",(victimCache_blocks?swaps:0));
        printf("h. Combined L1+VC miss rate: \t\t%.4f\n",(((float)__cache_read_miss__+__cache_write_miss__-swaps)/(__cache_reads__+__cache_writes__)));
        printf("i. Number of Write backs from L1/VC: \t%d\n",write_back);
        return (__cache_read_miss__+__cache_write_miss__+write_back-swaps);
    }
    if(level==2){
        printf("j. L2 read: \t\t\t\t%d\n",(num_cache_level==2?__cache_reads__:0));
        printf("k. L2 read miss: \t\t\t%d\n",(num_cache_level==2?__cache_read_miss__:0));
        printf("l. L2 write: \t\t\t\t%d\n",(num_cache_level==2?__cache_writes__:0));
        printf("m. L2 write miss: \t\t\t%d\n",(num_cache_level==2?__cache_write_miss__:0));
        printf("n. L2 miss rate: \t\t\t%.4f\n",(num_cache_level==2?((float)__cache_read_miss__/__cache_reads__):0));
        printf("o. Number of writebacks from L2: \t%d\n",(num_cache_level==2?write_back:0));
        return (__cache_read_miss__+__cache_write_miss__+write_back);
    }
    return 0;
}           
};


//--------------------------------------------------MAIN------------------------------------------------------------------
int main(int argc, char *argv[]){
    std::cout<<"===== Simulator configuration ====="<<std::endl;
    std::cout<<"L1 SIZE: \t\t"<<argv[1]<<std::endl;
    std::cout<<"L1 ASSOC: \t\t"<<argv[2]<<std::endl;
    std::cout<<"L1 BLOCKSIZE: \t\t"<<argv[3]<<std::endl;
    std::cout<<"VC_NUM_BLOCKS: \t\t"<<argv[4]<<std::endl;
    std::cout<<"L2 SIZE: \t\t"<<argv[5]<<std::endl;
    std::cout<<"L2 ASSOC: \t\t"<<argv[6]<<std::endl;
    std::cout<<"trace file: \t\t"<<argv[7]<<std::endl;

// Argument order: ./cache_sim <L1_SIZE> <L1_ASSOC> <L1_BLOCKSIZE> <VC_NUM_BLOCKS> <L2_SIZE> <L2_ASSOC> <trace_file>
    std::string filepath="../testbench/";
    ifstream file(filepath+argv[7]);
    std::string line;
    char op;
    char address[8];

    unsigned int l1_cache=atoi(argv[1]), l2_cache=atoi(argv[5]), l1_assoc=atoi(argv[2]), l2_assoc=atoi(argv[6]), vc_blocks=atoi(argv[4]) ,blocksize=atoi(argv[3]);    
    unsigned int l1_tag, l2_tag;



    unsigned int num_Cache_levels = (l2_cache!=0 ? 2:1);

    l1_tag= 32-(int)log2(l1_cache/l1_assoc);
    l2_tag= l2_cache?(32-(int)log2(l2_cache/l2_assoc)):0;

    L1 sim_l1(l1_cache,l1_assoc,blocksize,l1_tag,1,vc_blocks,num_Cache_levels);
    L1 sim_l2(l2_cache,l2_assoc,blocksize,l2_tag,2,0,num_Cache_levels);

    if (!file.is_open()) {
        cerr << "Error opening file." << std::endl;
        return -1;
    }
    
    int count=0;
    while (getline(file, line)) {
        sscanf(line.c_str(), "%c %s", &op, address);
        std::string add=address;             // Converting character array to String 

        sim_l1.put_addr(op,add);
        if(sim_l1.getSignal()==1)                           //Read
        {   if(num_Cache_levels==2)
                sim_l2.put_addr('r',add);
        }
        if(sim_l1.getSignal()==2){
            if(num_Cache_levels==2){                          //First Write Back and then Read
            sim_l2.put_addr('w',sim_l1.L2_write_addr());
            sim_l2.put_addr('r',add);
            }
        }
    }
    

    sim_l1.print_data();
    if(num_Cache_levels==2) sim_l2.print_data();

    unsigned memory_traffic_l1=0, memory_traffic_l2=0;
    std::cout<<"===== Simulation results (raw) ====="<<std::endl;
    memory_traffic_l1=sim_l1.print_simulation_result();
    memory_traffic_l2=sim_l2.print_simulation_result();
    printf("p. Total memory traffic \t\t%d\n",((num_Cache_levels==2)?memory_traffic_l2:memory_traffic_l1));
    
    //Instatiating CACTII
    std::cout<<std::endl<<"===== Simulation results (performance) ====="<<std::endl;
    float AccessTimeL1; float EnergyL1; float AreaL1;
    float AccessTimeL2; float EnergyL2; float AreaL2;
    float AccessTimeVC; float EnergyVC; float AreaVC;
    int cacti_l1=get_cacti_results(l1_cache, blocksize, l1_assoc, &AccessTimeL1, &EnergyL1, &AreaL1);
    int cacti_l2=get_cacti_results(l2_cache, blocksize, l2_assoc, &AccessTimeL2, &EnergyL2, &AreaL2);
    int cacti_vc=get_cacti_results(vc_blocks*blocksize, blocksize, vc_blocks, &AccessTimeVC, &EnergyVC, &AreaVC);
    if(l2_cache==0){
        //Configuration: L1+VC
        if( l1_cache && vc_blocks){
            if( !cacti_l1 && !cacti_vc ){
                float L1_HR=sim_l1.get_cache_hitratio();
                float totalaccess=sim_l1.get_totalaccess();             // reads+writes
                float totalmisses=sim_l1.get_misses();                  // read_miss + write_miss
                float VC_HR=sim_l1.get_VC_hitratio();
                float VC_AT=(AccessTimeVC>(1e-10)?AccessTimeVC:0.2);
                float AT= L1_HR*AccessTimeL1 + (1.0-L1_HR)*( VC_HR*VC_AT + (1.0-VC_HR)*(20.0 + (float)blocksize/16) );
                float TotalEnergy= (totalaccess+totalmisses)*EnergyL1 + 2*sim_l1.get_swaprequest()*EnergyVC + (totalmisses-sim_l1.get_swaps() + sim_l1.get_writeback())*0.05;
                std::cout<<"1. average access time: \t\t"<<AT<<std::endl;
                std::cout<<"2. energy-delay product: \t\t"<<TotalEnergy*AT<<std::endl;
                std::cout<<"3. total area: \t\t\t\t"<<AreaL1+AreaVC<<std::endl;
            }else{
                std::cout<<"CACTII Error"<<std::endl;
            }
        }//Configuration: L1
        else if (l1_cache){
            if(cacti_l1>0){
                std::cout<<"CACTII Error"<<std::endl;
            }
            else{
                float L1_HR=sim_l1.get_cache_hitratio();
                float AT= L1_HR*AccessTimeL1 + (1.0-L1_HR)*( 20.0 + (float)blocksize/16 );
                float totalaccess=sim_l1.get_totalaccess();             // reads+writes
                float totalmisses=sim_l1.get_misses();                  // read_miss + write_miss
                float totalEnergy= (totalaccess+totalmisses)*EnergyL1 + (totalmisses+sim_l1.get_writeback())*0.05; 
                std::cout<<"1. average access time: \t\t"<<AT<<std::endl;
                std::cout<<"2. energy-delay product: \t\t"<<totalEnergy*AT<<std::endl;
                std::cout<<"3. total area: \t\t\t\t"<<AreaL1<<std::endl;
            }
        } 
    }else{  //Configuration: L1+VC+L2
            if( l1_cache && vc_blocks){
            if( !cacti_l1 && !cacti_vc && !cacti_l2 ){
                float L1_HR=sim_l1.get_cache_hitratio();
                float L2_HR=sim_l2.get_cache_hitratio();
                float totalaccess_l1=sim_l1.get_totalaccess();             // reads+writes
                float totalmisses_l1=sim_l1.get_misses();                  // read_miss + write_miss
                float totalaccess_l2=sim_l2.get_totalaccess();             // reads+writes
                float totalmisses_l2=sim_l2.get_misses();                  // read_miss + write_miss
                float VC_HR=sim_l1.get_VC_hitratio();
                float VC_AT=(AccessTimeVC>(1e-10)?AccessTimeVC:0.2);
                float AT= L1_HR*AccessTimeL1 + (1.0000000-L1_HR)*( AccessTimeL1 + VC_HR*VC_AT 
                                        +(1.0000000-VC_HR)*(L2_HR*AccessTimeL2 + AccessTimeL1 
                                        +(1.0000000-L2_HR)*(AccessTimeL1 + AccessTimeL2 + AccessTimeVC + 20.0 + (float)blocksize/16)) );
                float TotalEnergy= (totalaccess_l1+totalmisses_l1)*EnergyL1 + 2*sim_l1.get_swaprequest()*EnergyVC 
                                    +(totalaccess_l2+totalmisses_l2)*EnergyL2
                                    + (totalmisses_l2 + sim_l2.get_writeback())*0.05;
                std::cout<<"1. average access time: \t\t"<<AT<<std::endl;
                std::cout<<"2. energy-delay product: \t\t"<<TotalEnergy*AT<<std::endl;
                std::cout<<"3. total area: \t\t\t\t"<<AreaL1+AreaVC+AreaL2<<std::endl;
            }else{
                std::cout<<"CACTII Error"<<std::endl;
            }
        }//Configuration: L1+L2
        else{   
                if( !cacti_l1 && !cacti_l2 ){
                float L1_HR=sim_l1.get_cache_hitratio();
                float L2_HR=sim_l2.get_cache_hitratio();
                float totalaccess_l1=sim_l1.get_totalaccess();             // reads+writes
                float totalmisses_l1=sim_l1.get_misses();                  // read_miss + write_miss
                float totalaccess_l2=sim_l2.get_totalaccess();             // reads+writes
                float totalmisses_l2=sim_l2.get_misses();                  // read_miss + write_miss
                std::cout<<AccessTimeL1<<" "<<" "<<AccessTimeL2<<" "<<" "<<std::endl;
                float AT= L1_HR*AccessTimeL1 + (1.0000000-L1_HR)*(AccessTimeL1+L2_HR*(AccessTimeL2)
                                        +(1.0000000-L2_HR)*(AccessTimeL2+AccessTimeL1 + 20.0 + (float)blocksize/16));
                float TotalEnergy= (totalaccess_l1+totalmisses_l1)*EnergyL1
                                    +(totalaccess_l2+totalmisses_l2)*EnergyL2
                                    + (totalmisses_l2 + sim_l2.get_writeback())*0.05;
                std::cout<<"1. average access time: \t\t"<<AT<<std::endl;
                std::cout<<"2. energy-delay product: \t\t"<<TotalEnergy*AT<<std::endl;
                std::cout<<"3. total area: \t\t\t\t"<<AreaL1+AreaL2<<std::endl;
            }else{
                std::cout<<"CACTII Error"<<std::endl;
            }
        }

    }
    file.close();
    return 0;
}