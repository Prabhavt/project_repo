// Author:  Prabhav Talukdar
// Cache Simulator : L1+VC
#include<iostream>
#include<math.h>
#include<string>
#include<fstream>
#include<stdlib.h>
#include<vector>
#include"cache.h"

//-------------------------------------------Address Structure----------------------------------------------------------------------
//                      Bit:                 <---log2(CacheSize/Assoc)--->         
//                              31________________________________________0
//                              |  Tag Bits |   Set No.    | Block Offset |
//                              --------------------------------------------   
//-----------------------------------------------------------------------------------------------------------------------------------

//Sizes are in bytes
#define L1_CACHE               1024      // Number of Set x Associativity x Block Offset
#define L1_ASSOC               2         // Associativity
#define BLOCKSIZE              16        // Size in Bytes           
#define L1_TAG                 32-(int)log2(L1_CACHE/L1_ASSOC)

#define VC_blocks              16        // Number of Blocks in Victim Cache

#define L2_CACHE               0         // Number of Set x Associativity x Block Offset
#define L2_ASSOC               0         // Associativity           
#define L2_TAG                 L2_CACHE?(32-(int)log2(L2_CACHE)-(int)log2(L2_ASSOC)):0
using namespace std;

//--------------------------------------------Cache Class----------------------------------------------------------------------

class L1{
    string              **mem;                         
    string              **__cachefulladdr__;
    string              *victim;
    int                 **valid;
    int                 **dirty;
    int                 *victim_dirty;
    int                 **LRU_count;
    int                 *victim_LRU;
    unsigned int        set_no,display_setno;
    unsigned int        L1_read_miss;
    unsigned int        L1_reads;
    unsigned int        L1_write_miss;
    unsigned int        L1_writes;
    unsigned int        cache, assoc, blocksize, tag, level, victimCache_blocks;
    string              addr_bin;
    string              addr_hex;                        
    string              writeback_address;
    char                op;
    unsigned int        read_L2;           // L1 -> L2  or  L2 -> Memory
    unsigned int        write_back_L2;     //signals
    unsigned int        write_back;        //count of total write backs from L1
    unsigned int        swaps;
    unsigned int        swap_request;
public:

//--------------------------Initializing Cache information----------------------------------------------------------------------------------------
    L1(int C, int A, int B, int T, int L, int V){
        cache               =C;
        assoc               =A;
        blocksize           =B;
        tag                 =T;
        level               =L;
        victimCache_blocks  =V;

        mem                 = new string *[(cache/(assoc*blocksize))];
        __cachefulladdr__   = new string *[(cache/(assoc*blocksize))];
        valid               = new int    *[(cache/(assoc*blocksize))];
        dirty               = new int    *[(cache/(assoc*blocksize))];
        LRU_count           = new int    *[(cache/(assoc*blocksize))];
        
        //Assign Victim Cache if level = 1
        if(L==1 && V !=0){
            victim          = new string [V];
            victim_LRU      = new int [V];
            victim_dirty    = new int [V];
            swaps           = 0;
            swap_request    = 0;
            for(int i=0; i<V; i++){
                victim_LRU[i]   =V-1;
                victim[i]       ="-1";
                victim_dirty[i] =0;
        }
        }

        for(int i=0; i<((cache/(assoc*blocksize)));i++){
            mem[i]          = new string[assoc];
            __cachefulladdr__[i]= new string[assoc];
            valid[i]        = new int[assoc];
            dirty[i]        = new int[assoc];
            LRU_count[i]    = new int[assoc];
        }

        for(int i=0;i<(cache/(assoc*blocksize));i++){
            for(int j=0; j<assoc;j++){
            mem[i][j]       ="-1";
            valid[i][j]     =0;              //valid=0 : Not Valid          valid=1 : Valid
            LRU_count[i][j] =assoc-1;
            dirty[i][j]     =0;              //dirty=0 : Not Modified       dirty=1 : Modified
            }
        }

        L1_read_miss        =0;
        L1_reads            =0;
        L1_writes           =0;
        L1_write_miss       =0;
        read_L2             =0;
        write_back_L2       =0;   
        write_back          =0;                  
    }

    void put_addr(char operation, string address){
        addr_hex            =address;
        op                  =operation;
        unsigned int add_dec=hex_to_dec(address);
        addr_bin            =dec_to_bin(add_dec);
        unsigned int set_no =bin_to_dec( addr_bin, ((int)log2(blocksize)) , ((int)log2(cache/assoc))-1  );
        display_setno       =set_no;
        read_L2             =0; 
        write_back_L2       =0;
    //---------------------------Read Operation------------------------------------------------------------------------------------------------------

            if(op=='r'){
                
                //Increment number of Reads 
                L1_reads+=1;

                //Check if any Invalid block is present in Cache(L1/L2) set
                int empty_block =0;
                for(int i=0; i<assoc; i++){
                    if(valid[set_no][i]==0){
                        empty_block=1;
                        break;
                    }                            
                }
                
                
                for(int i=0; i<assoc; i++){
                    string mem_FA_tag  =hex_to_bin(__cachefulladdr__[set_no][i]).substr((int)log2(blocksize),32-(int)log2(blocksize));
                    string addr_FA_tag =hex_to_bin(address).substr((int)log2(blocksize),32-(int)log2(blocksize));
                    
                    //----------------------Read Hit-----------------------------------------------
                    //Comparing (tag+index) bits of Requested Address and Cache block address
                    if( !addr_FA_tag.compare(mem_FA_tag) ){
                        //-------LRU Update------------
                        LRU_update(set_no, i); 
                        break; 
                    }
            
                    //------------------Read Miss-------------------
                    if(i==(assoc-1)){
                        
                        //Incrementing L1 Read miss count
                        L1_read_miss+=1;

                        //---------------Read Miss but Cache set have invalid Cache Line--------------------------- 
                        if(empty_block){
                            //-----------------------------------Cold Miss------------------------------- 
                            //---------------------Bring Data to Cold Blocks from Next Level--------------
                            //------------------------Read Signal to next Level--------------------------- 

                            //Signaling Next level cache/memory for Read
                            read_L2=1;
                            //Reseting empty block flag
                            empty_block=0;
                            for(int j=0; j<assoc; j++){
                                if(valid[set_no][j]==0){
                                    mem[set_no][j]                  =addr_bin.substr((int)log2(cache/assoc),tag);
                                    __cachefulladdr__[set_no][j]    =address;

                                    valid[set_no][j]                =1;
                                    dirty[set_no][j]                =0;
                                    //-------LRU Update------------
                                    LRU_update(set_no, j); 
                                    break;
                                }
                            }
                            break;
                        }
                        //---------------Read Miss but all valid blocks in cache set---------------------------   
                        else if(victimCache_blocks && ~empty_block)
                        {
                            //-------------------If Victim Cache is present----------------------------------------
                            //---------------If L1 cache with Victim Cache present---------------------------------
                            //Increment Swap request
                            swap_request++;
                            int lru_pos         =-1;
                            //-------LRU position in cache------
                            for(int k=0; k<assoc; k++){
                                if(LRU_count[set_no][k]==assoc-1){
                                    lru_pos             =k;
                                    break;
                                }
                            }
                                bool victim_hit = false;
                                int victim_pos  =-1;
                                
                                for(int k=0; k<victimCache_blocks; k++){
                                    string vc_tag                    =hex_to_bin(victim[k]).substr((int)log2(blocksize),32-(int)log2(blocksize));
                                    string addr_fullyassociative_tag =hex_to_bin(address).substr((int)log2(blocksize),32-(int)log2(blocksize));
                                    if(!addr_fullyassociative_tag.compare(vc_tag)){
                                        victim_hit=true;
                                        victim_pos=k;
                                        break;
                                    }
                                }
                                
                                //--------Victim Cache Hit-----------
                                if(victim_hit){
                                    unsigned int victim_hex_dec =hex_to_dec(victim[victim_pos]);
                                    string victim_bin           =dec_to_bin(victim_hex_dec);
                                    mem[set_no][lru_pos]        =victim_bin.substr((int)log2(cache/assoc),tag);
                                    victim[victim_pos].swap(__cachefulladdr__[set_no][lru_pos]);
                                    swap(dirty[set_no][lru_pos],victim_dirty[victim_pos]);
                                    swaps++;

                                    LRU_VC_update(victim_pos);
                                    //-------LRU Update------------
                                    LRU_update(set_no, lru_pos); 

                                }
                                //--------Victim Cache Miss------------
                                else{
                                    //----Read from next level---------
                                    read_L2=1;

                                    bool invalid_victim_block=false;
                                    int victimpos=-1;

                                    for(int k=0; k<victimCache_blocks; k++){
                                        if(victim[k]=="-1"){
                                            invalid_victim_block=true;
                                            victimpos=k;
                                            break;
                                        }
                                    }
                                    if(invalid_victim_block){
                                        victim[victimpos]                =__cachefulladdr__[set_no][lru_pos];
                                        victim_dirty[victimpos]          = dirty[set_no][lru_pos];
                                        
                                        mem[set_no][lru_pos]              = addr_bin.substr((int)log2(cache/assoc),tag);
                                        __cachefulladdr__[set_no][lru_pos]= address; 
                                        dirty[set_no][lru_pos]            = 0;    //xxxxxxxxxxdirty of l1xxxxxxxxxxxxxxxxxxx//
                                        valid[set_no][lru_pos]            = 1;
                                        

                                        LRU_VC_update(victim_pos);
                                    //-------LRU Update------------
                                        LRU_update(set_no, lru_pos); 
                                    }
                                    //------Victim Cache Set Full------------
                                    else{
                                        for(int k=0; k< victimCache_blocks; k++){
                                            if(victim_LRU[k]==(victimCache_blocks-1)){
                                                //------Dirty Block has to be written back---------
                                                if(victim_dirty[k]){
                                                    write_back++;
                                                    write_back_L2=1;
                                                    writeback_address =victim[k];
                                                }

                                                victim[k]                                 = __cachefulladdr__[set_no][lru_pos];
                                                victim_dirty[k]                           = dirty[set_no][k];
                                                mem[set_no][lru_pos]                      = addr_bin.substr((int)log2(cache/assoc),tag);
                                                __cachefulladdr__[set_no][lru_pos]        = address;
                                                valid[set_no][lru_pos]                    = 1;
                                                dirty[set_no][lru_pos]                    = 0;
                                                

                                                LRU_VC_update(k);
                                                //-------LRU Update------------
                                                LRU_update(set_no, lru_pos); 
                                                break;
                                            }
                                        }
                                    }
                                }
                        }
                        else{
                            L1_read_miss+=1;
                            read_L2=1;
                            for(int j=0; j<assoc; j++){
                                if( LRU_count[set_no][j] == (assoc-1) ){                         
                                    if(dirty[set_no][j]){
                                        write_back_L2                   =1;
                                        write_back++;
                                        writeback_address               = __cachefulladdr__[set_no][j];                             
                                    }
                                    mem[set_no][j]                      = addr_bin.substr((int)log2(cache/assoc),tag);
                                    __cachefulladdr__[set_no][j]        = address;
                                    valid[set_no][j]                    = 1;
                                    dirty[set_no][j]                    = 0;
                                
                                    //-------LRU Update------------
                                    LRU_update(set_no, j); 
                                    break;
                                }
                            }
                        }
                    }
                }
            }
//-------------------------------------------------------Write Operation------------------------------------------------------------------------------------------------------

            if(op=='w'){
                L1_writes++;
                int empty_block  =0;
                //---Checking Line capacity---
                for(int i=0; i<assoc; i++){
                    if(valid[set_no][i]==0){
                        empty_block=1;
                        break;
                    }                            
                }

                for(int i=0; i<assoc; i++){

                    string mem_FA_tag  =hex_to_bin(__cachefulladdr__[set_no][i]).substr((int)log2(blocksize),32-(int)log2(blocksize));
                    string addr_FA_tag =hex_to_bin(address).substr((int)log2(blocksize),32-(int)log2(blocksize));
                    //----------------------Write Hit-----------------------------------------------
                    if( !addr_FA_tag.compare(mem_FA_tag) ){
                        //------Dirty Bit Set----------
                        dirty[set_no][i]    = 1;
                        valid[set_no][i]    = 1;        
                        //-------LRU Update------------
                            LRU_update(set_no, i); 
                        break;
                }

                //--------------Write Miss-------------------------------------
                if((i==(assoc-1)))
                {
                    L1_write_miss   +=1;

                    //-----No Write Allocate if Invalid Block Present--------
                    if(empty_block){
                        read_L2          =1;
                        empty_block      =0;

                        for(int j=0; j<assoc; j++){
                            if(valid[set_no][j]==0){
                                
                                mem[set_no][j]               =addr_bin.substr((int)log2(cache/assoc),tag);
                                __cachefulladdr__[set_no][j] =address;

                                valid[set_no][j]=1;
                                dirty[set_no][j]=1;                 //Empty block allocated
                                  
                                //-------LRU Update------------
                                LRU_update(set_no, j); 
                                break;
                            }
                        }            
                    }
                    //-------------------If Victim Cache is present--------------------------------
                    //---------------Write Miss but all valid lines in set---------------------------   
                    else if(victimCache_blocks && ~empty_block)
                    {
                        swap_request++;
                        int lru_pos         =-1;
                        //-------LRU position in cache------
                        for(int k=0; k<assoc; k++){
                            if(LRU_count[set_no][k]==(assoc-1)){
                                lru_pos     =k;
                                break;
                            }
                        }
                        
                        //-----If L1 cache with Victim Cache present------
                        if( (level==1) && (victimCache_blocks) ){
                            bool victim_hit = false;
                            int victim_pos  =-1;
                            
                            for(int k=0; k<victimCache_blocks; k++){
                                    string vc_tag                    =hex_to_bin(victim[k]).substr((int)log2(blocksize),32-(int)log2(blocksize));
                                    string addr_fullyassociative_tag =hex_to_bin(address).substr((int)log2(blocksize),32-(int)log2(blocksize));
                                    if(!addr_fullyassociative_tag.compare(vc_tag)){
                                    victim_hit=true;
                                    victim_pos=k;
                                    break;
                                }
                            }
                                
                            //--------Victim Cache Hit-----------
                            if(victim_hit){
                                unsigned int victim_hex_dec =hex_to_dec(victim[victim_pos]);
                                string victim_bin           =dec_to_bin(victim_hex_dec);
                                mem[set_no][lru_pos]        =victim_bin.substr((int)log2(cache/assoc),tag);
                                victim[victim_pos].swap(__cachefulladdr__[set_no][lru_pos]);
                                victim_dirty[victim_pos] = dirty[set_no][lru_pos];
                                dirty[set_no][lru_pos]=1;
                                swaps++;

                                LRU_VC_update(victim_pos);
                                //-------LRU Update------------
                                LRU_update(set_no, lru_pos); 
                            }
                            //--------Victim Cache Miss------------
                            else{
                                //----Read from next level and Write Allocate---------
                                read_L2         =1;

                                bool invalid_victim_block=false;
                                int victimpos  =-1;

                                for(int k=0; k<victimCache_blocks; k++){
                                    if(victim[k]=="-1"){
                                        invalid_victim_block=true;
                                        victimpos=k;
                                        break;
                                    }
                                }
                                if(invalid_victim_block){
                                    victim[victimpos]                =__cachefulladdr__[set_no][lru_pos];
                                    victim_dirty[victimpos]          = dirty[set_no][lru_pos];
                                    
                                    mem[set_no][lru_pos]              = addr_bin.substr((int)log2(cache/assoc),tag);
                                    __cachefulladdr__[set_no][lru_pos]= address; 
                                    dirty[set_no][lru_pos]            = 1;
                                    valid[set_no][lru_pos]            = 1;
                                   
                                    //------LRU Update------
                                    LRU_VC_update(victimpos);
                                    LRU_update(set_no, lru_pos); 
                                }

                                //----------Victim Cache Not Present-------
                                //-----------Victim Cache Set Full---------------
                                else{
                                    for(int k=0; k< victimCache_blocks; k++){
                                        if(victim_LRU[k]==(victimCache_blocks-1)){
                                            //------Dirty Block has to be written back---------
                                            if(victim_dirty[k]){
                                                write_back++;
                                                write_back_L2                         =1;
                                                writeback_address                     =victim[k];
                                            }
                                            victim[k]                                 = __cachefulladdr__[set_no][lru_pos];
                                            victim_dirty[k]                           = dirty[set_no][lru_pos];
                                            mem[set_no][lru_pos]                      = addr_bin.substr((int)log2(cache/assoc),tag);
                                            __cachefulladdr__[set_no][lru_pos]        = address;
                                            valid[set_no][lru_pos]                    = 1;
                                            dirty[set_no][lru_pos]                    = 1;

                                            
                                            //------Victim Cache LRU update----------
                                            LRU_VC_update(k);
                                            //-------LRU Update------------
                                            LRU_update(set_no, lru_pos); 
                                            break;
                                        }
                                    }
                                }
                            }
                       }
                    }
                    else{
                        L1_write_miss   +=1;
                        read_L2         =1;
                        for(int j=0; j<assoc; j++){
                            //Replace The Lease Recently Used Dirty Block
                            if(LRU_count[set_no][j] == assoc-1){
                                if(dirty[set_no][j]){
                                    //Write allocate and Write in L1_CACHE
                                    write_back++;
                                    write_back_L2=1;
                                    writeback_address = __cachefulladdr__[set_no][j];                                                               
                                }

                                dirty[set_no][j]=1; 
                                mem[set_no][j] = addr_bin.substr((int)log2(cache/assoc),tag);
                                __cachefulladdr__[set_no][j]=address;

                                //-------LRU Update------------
                                LRU_update(set_no, j); 
                                break;
                            }                
                        }
                    }
                }
            }
                
        }
    }

void LRU_VC_update(int pos){
    int lru_count= victim_LRU[pos];
    for(int i=0; i<victimCache_blocks; i++){
        if(victim_LRU[i] < lru_count)
            victim_LRU[i]++;
    }
    victim_LRU[pos]=0;
}

//---------------------Updating Least Recently Used block Counter--------------------------------------
void LRU_update(int set, int pos){
    int write_lru_count=LRU_count[set][pos];
    //-------LRU Update------------
    for (int k=0; k<assoc; k++){
        if(LRU_count[set][k] < write_lru_count)
        LRU_count[set][k] +=1;
    }
    LRU_count[set][pos]= 0;
}
//--------------------Signalling next Level of Memory---------------------------------------------------
int getSignal(){
    if(write_back_L2 && read_L2){
        return 2;
    }
    else if(read_L2 && !(write_back_L2)){
        return 1;
    }
    return 0;
}

string L2_write_addr(){
    return writeback_address;
}

//--------------------Sorting Blocks wrt LRU to display them in order----------------------------

void sort(string cacheline[], int D[], int lru_count[]){
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

//-------------------------------------Print CACHE Data------------------------------------------
void print_data(){
        cout<<endl<<endl;
        cout<<"Configuration: "<<endl;
        cout<<"Cache Size: "<<cache<<" Associativity: "<<assoc<<" BlockSize: "<<blocksize<<endl<<endl;
        for(int i=0;i<(cache/(assoc*blocksize));i++){
            printf("set no. %3d %4.4s",i,"  | ");
            sort(mem[i],dirty[i],LRU_count[i]);
        for(int j=0; j<assoc;j++){
            if(mem[i][j]!="-1")
            {   
                string hex_add=bin_to_hex(mem[i][j]);
                for(int i=hex_add.length()-1; i>=0; i--){
                    printf("%c",hex_add[i]);
                }
                if(dirty[i][j]){
                    cout<<" D";
                }else{
                    cout<<"  ";
                }
            }
            else{
                cout<<mem[i][j];
            }
            printf("%3.1s","|");
        }
        cout<<endl;
        }
        cout<<"-------------------------------------"<<endl;
        cout<<"L"<<level<<" read: "<<L1_reads<<endl;
         cout<<"L"<<level<<" read miss: "<<L1_read_miss<<endl;
        cout<<"L"<<level<<" write: "<<L1_writes<<endl;
        cout<<"L"<<level<<" write miss: "<<L1_write_miss<<endl;
        cout<<"Write backs from L"<<level<<"/VC: "<<write_back<<endl;
        cout<<"Number of swaps "<<swaps<<endl;
        cout<<"Number of swap request: "<<swap_request<<endl;

    if(victimCache_blocks){
        cout<<"------VC Data----------"<<endl;
        for(int i=0; i<victimCache_blocks; i++){
            unsigned int addrdec=hex_to_dec(victim[i]);
            string addrbin      =dec_to_bin(addrdec);
            string revAddr      =bin_to_hex(addrbin.substr((int)log2(blocksize),32-(int)log2(blocksize)));
            for(int j=revAddr.length()-1; j>=0; j--)
            {    cout<<revAddr[j];}
            if(victim_dirty[i])
                cout<<" D\n";
            else
                cout<<" \n";
        }
        cout<<endl;         
    }
    cout<<endl;
}
};


//--------------------------------------------------MAIN------------------------------------------------------------------
int main(){
    cout<<"--------------Cache Simulator-----------------"<<endl;
    cout<<"----------Author: Prabhav Talukdar------------"<<endl;
    cout<<"----------------------------------------------"<<endl;
    string filepath="/home/prabhav/LocalDisk/EE23M053/CS6600_COA/Project/project1_cachesim/Assignment1/Assignment_files/gcc_trace.txt";
    ifstream file(filepath);
    string line;
    char op;
    char address[8];
 
    L1 sim_l1(L1_CACHE,L1_ASSOC,BLOCKSIZE,L1_TAG,1,VC_blocks);
    //L1 sim_l2(L2_CACHE,L2_ASSOC,BLOCKSIZE,L2_TAG,2,0);

    if (!file.is_open()) {
        cerr << "Error opening file." << endl;
        return -1;
    }
    
    int count=0;
    while (getline(file, line)) {
        sscanf(line.c_str(), "%c %s", &op, address);
        string add=address;             // Converting character array to String 
        sim_l1.put_addr(op,add);
        // if(sim_l1.getSignal()==1)                           //Read
        // {   if(L2_CACHE)
        //     sim_l2.put_addr('r',add);
        // }
        // if(sim_l1.getSignal()==2){
        //     if(L2_CACHE){                          //First Write Back and then Read
        //     sim_l2.put_addr('w',sim_l1.L2_write_addr());
        //     sim_l2.put_addr('r',add);
        //     }
        // }
        //count++;
    }

    sim_l1.print_data();
    //sim_l2.print_data();

    file.close();
    return 0;
}