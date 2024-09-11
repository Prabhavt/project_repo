// Author:  Prabhav Talukdar
// Cache Simulator : L1 

#include<iostream>
#include<math.h>
#include <string>
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

#define L2_CACHE               8192      // Number of Set x Associativity x Block Offset
#define L2_ASSOC               4         // Associativity           
#define L2_TAG                 32-(int)log2(L2_CACHE/L2_ASSOC)
using namespace std;

//--------------------------------------------Cache Class----------------------------------------------------------------------

class L1{
    string              **mem;                         
    string              **__cachefulladdr__;
    int                 **valid;
    int                 **dirty;
    int                 **LRU_count;
    unsigned int        set_no,display_setno;
    unsigned int        L1_read_miss;
    unsigned int        L1_reads;
    unsigned int        L1_write_miss;
    unsigned int        L1_writes;
    unsigned int        cache, assoc, blocksize, tag, level;
    string              addr_bin;
    string              addr_hex;                        
    string              writeback_address;
    char                op;
    unsigned int        read_L2;           // L1 -> L2  or  L2 -> Memory
    unsigned int        write_back_L2;     //signals
    unsigned int        write_back;        //count of total write backs from L1
public:

//--------------------------Initializing Cache information----------------------------------------------------------------------------------------
    L1(int C, int A, int B, int T, int L){
        cache               =C;
        assoc               =A;
        blocksize           =B;
        tag                 =T;
        level               =L;

        mem              =  new string *[(cache/(assoc*blocksize))];
        __cachefulladdr__=  new string *[(cache/(assoc*blocksize))];
        valid            =  new int *[(cache/(assoc*blocksize))];
        dirty            =  new int *[(cache/(assoc*blocksize))];
        LRU_count        =  new int *[(cache/(assoc*blocksize))];

        for(int i=0; i<((cache/(assoc*blocksize)));i++){
            mem[i]         = new string[assoc];
            __cachefulladdr__[i]= new string[assoc];
            valid[i]       = new int[assoc];
            dirty[i]       = new int[assoc];
            LRU_count[i]   = new int[assoc];
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
                
                L1_reads+=1;
                int empty_block =0;


                for(int i=0; i<assoc; i++){
                    if(valid[set_no][i]==0)
                        empty_block=1;
                    break;                            
                }
                
                for(int i=0; i<assoc; i++){

                    //----------------------Read Hit-----------------------------------------------
                    if( mem[set_no][i] == addr_bin.substr((int)log2(cache/assoc),tag) ){
                        //-------LRU Update------------
                        LRU_update(set_no, i); 
                        break; 
                    }
            
                    //------------------Read Miss-------------------
                    if(i==(assoc-1)){
                        L1_read_miss+=1;
                        read_L2=1;                                                  // Signaling next level for read

                        if(empty_block==1){
                            empty_block=0;
                            //-----------------------------------Cold Miss------------------------------- 
                            //---------------------Bring Data to Cold Blocks from Next Level--------------
                            //------------------------Read Signal to next Level--------------------------- 
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
                        }  
                        else
                        {
                        for(int j=0; j<assoc; j++){
                            if( LRU_count[set_no][j] == (assoc-1) ){                         
                                if(dirty[set_no][j]==1){
                                    write_back_L2                   =1;
                                    write_back++;
                                    writeback_address               = __cachefulladdr__[set_no][j];                             
                                }
                                mem[set_no][j]                      = addr_bin.substr((int)log2(cache/assoc),tag);
                                __cachefulladdr__[set_no][j]        =address;
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
    //---------------------------Write Operation------------------------------------------------------------------------------------------------------

            if(op=='w'){
                L1_writes       +=1;
                int empty_block  =0;
                
                //---Checking Line capacity---
                for(int i=0; i<assoc; i++){              
                if(valid[set_no][i]==0)
                    empty_block =1;
                    break;                            
                }

                for(int i=0; i<assoc; i++){

                //--------------Write Hit-------------------------------------
                if( mem[set_no][i] == addr_bin.substr((int)log2(cache/assoc),tag) ){
                        //------Dirty Bit Set----------
                        dirty[set_no][i]    = 1;
                        valid[set_no][i]    = 1;        
                        //-------LRU Update------------
                            LRU_update(set_no, i); 
                        break;
                }

                //--------------Write Miss and Write Back-------------------------------------
                if( (mem[set_no][i] != addr_bin.substr((int)log2(cache/assoc),tag)) && (i==(assoc-1)) )
                {
                    L1_write_miss   +=1;
                    read_L2          =1;

                    //-----No Write Allocate if Invalid Block Present--------
                    if(empty_block==1){
                        empty_block  =0;
                        for(int j=0; j<assoc; j++){
                            if(valid[set_no][j]==0){
                                
                                mem[set_no][j] = addr_bin.substr((int)log2(cache/assoc),tag);
                                __cachefulladdr__[set_no][j]=address;

                                valid[set_no][j]=1;
                                dirty[set_no][j]=1;                 //Empty block allocated  
                                //-------LRU Update------------
                                LRU_update(set_no, j); 
                                break;
                            }
                        }
                    //--------------------Write Allocate--------------------------               
                    }else{
                        for(int j=0; j<assoc; j++){
                            //Replace The Lease Recently Used Dirty Block
                            if(LRU_count[set_no][j] == assoc-1){
                                if(dirty[set_no][j]==1){
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
        cout<<"Write backs from L"<<level<<": "<<write_back<<endl;
}
};


//--------------------------------------------------MAIN------------------------------------------------------------------
int main(){
    cout<<"--------------Cache Simulator-----------------"<<endl;
    cout<<"----------Author: Prabhav Talukdar------------"<<endl;
    cout<<"----------------------------------------------"<<endl;
    string filepath="/home/prabhav/LocalDisk/EE23M053/CS6600 COA/Project/project1_cachesim/Assignment1/Assignment_files/gcc_trace.txt";
    ifstream file(filepath);
    string line;
    char op;
    char address[8];
    L1 sim_l1(L1_CACHE,L1_ASSOC,BLOCKSIZE,L1_TAG,1);
    L1 sim_l2(L2_CACHE,L2_ASSOC,BLOCKSIZE,L2_TAG,2);

    if (!file.is_open()) {
        cerr << "Error opening file." << endl;
        return -1;
    }
    
    int count=0;

    while (getline(file, line) ) {
        sscanf(line.c_str(), "%c %s", &op, address);
        string add=address;             // Converting character array to String 
        
        sim_l1.put_addr(op,add);

        if(sim_l1.getSignal()==1)                           //Read
        {   
            sim_l2.put_addr('r',add);
        }
        if(sim_l1.getSignal()==2){                          //First Write Back and then Read
            sim_l2.put_addr('w',sim_l1.L2_write_addr());
            sim_l2.put_addr('r',add);
        }
        //count++;
    }
    sim_l1.print_data();
    sim_l2.print_data();

    file.close();
    return 0;
}