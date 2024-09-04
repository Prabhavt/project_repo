// Author:  Prabhav Talukdar
// Cache Simulator 

#include<iostream>
#include<math.h>
#include<fstream>
#include<stdlib.h>
#include<string.h>
//-------------------------------------------Address Structure----------------------------------------------------------------------
//                      Bit:                 <---log2(CacheSize/Assoc)--->         
//                              31________________________________________0
//                              |  Tag Bits |   Set No.    | Block Offset |
//                              --------------------------------------------   
//-----------------------------------------------------------------------------------------------------------------------------------

//Sizes are in bytes
#define CACHE               1024      // Number of Set x Associativity x Block Offset
#define ASSOC               2         // Associativity
#define BLOCKSIZE           16        // Size in Bytes           
#define TAG                 32-(int)log2(CACHE/ASSOC)
using namespace std;


//--------------------------------------------Function Definations-------------------------------------------------------------
unsigned int bin_to_dec(string bin, int start, int end);
string dec_to_bin(unsigned int addr_dec);
int hex_to_dec(string str_addr);

//--------------------------------------------Cache Class----------------------------------------------------------------------

class L1{
    string mem[CACHE/(ASSOC*BLOCKSIZE)][ASSOC];
    int valid[CACHE/(ASSOC*BLOCKSIZE)][ASSOC];
    int dirty[CACHE/(ASSOC*BLOCKSIZE)][ASSOC];
    int LRU_count[CACHE/(ASSOC*BLOCKSIZE)][ASSOC];
    unsigned int set_no;
    unsigned int L1_read_miss;
    unsigned int L1_reads;
    unsigned int L1_write_miss;
    unsigned int L1_writes;

protected:
    string addr_hex;
    string addr_bin;
    char op;
    unsigned int read_next_level;           // L1 -> L2  or  L2 -> Memory
    unsigned int write_back_signal;

public:

//--------------------------Initializing Cache information----------------------------------------------------------------------------------------
    L1(){
        for(int i=0;i<(CACHE/(ASSOC*BLOCKSIZE));i++){
            for(int j=0; j<ASSOC;j++){
            mem[i][j]="-1";
            valid[i][j]=0;
            LRU_count[i][j]=0;
            dirty[i][j]=0;
            }
        }
        L1_read_miss=0;
        L1_reads=0;
        L1_writes=0;
        L1_write_miss=0;
        read_next_level=0;
    }

    void get_addr(char operation, string address){
        addr_hex=address;
        op= operation;
        unsigned int add_dec= hex_to_dec(address);
        addr_bin=dec_to_bin(add_dec);
        unsigned int set_no=bin_to_dec( addr_bin, ((int)log2(BLOCKSIZE)) , ((int)log2(CACHE/ASSOC))-1  );

//---------------------------Read Operation------------------------------------------------------------------------------------------------------

        if(op=='r'){
            
            L1_reads+=1;
            read_next_level=0;
            int read_miss_flag=0;
            
            for(int i=0; i<ASSOC; i++){
                
                // Cold Miss 
                //---------------------Bring Data to Cold Blocks from Next Level-------------- 
                if(valid[set_no][i]==0){
                    L1_read_miss+=1;
                    read_next_level=1;
                    mem[set_no][i]= addr_bin.substr((int)log2(CACHE/ASSOC),TAG);
                    valid[set_no][i]=1;
                    LRU_count[set_no][i]=i;
                    read_miss_flag=1;
                    break;
                }

                //----------------------Read Hit-----------------------------------------------
                if( mem[set_no][i] == addr_bin.substr((int)log2(CACHE/ASSOC),TAG) ){
                    read_miss_flag=1;
                    int hit_lru_count=LRU_count[set_no][i];

                    //------------------LRU Update---------------------------------------------
                    for(int j=0; j< ASSOC; j++){
                        if(LRU_count[set_no][j] < hit_lru_count)
                            LRU_count[set_no][j] +=1;
                    }
                    //Setting Cache Hit Element LRU count to 0 
                    LRU_count[set_no][i]=0;
                    break; 
                }
            }
            //Read Miss
            //Data brought in from next level of memory heirarchy 
            if(read_miss_flag==0){  
                for(int i=0; i<ASSOC; i++){
                    if( LRU_count[set_no][i] == (ASSOC-1) ){
                        L1_read_miss+=1;                                                    // Miss on current level
                        read_next_level=1;                                                  // Signaling next level for read
                        mem[set_no][i] = addr_bin.substr((int)log2(CACHE/ASSOC),TAG);
                        int miss_lru_count=LRU_count[set_no][i];

                        //-----------LRU Update--------------------
                        for (int j=0; j<ASSOC; j++){
                            if(LRU_count[set_no][j] < miss_lru_count)
                                LRU_count[set_no][j] +=1;
                        }
                        LRU_count[set_no][i]= 0;
                    break;
                    }
                }
            }
        }
//---------------------------Write Operation------------------------------------------------------------------------------------------------------

        else{
            L1_writes+=1;
            write_back_signal=0;
            for(int i=0; i<ASSOC; i++){
            
            //--------------Write Miss in Cold Block----------------------
            if(valid[set_no][i]==0){
                L1_write_miss+=1;
                read_next_level+=1;
                mem[set_no][i]= addr_bin.substr((int)log2(CACHE/ASSOC),TAG);
                valid[set_no][i]=1;
                LRU_count[set_no][i]=i;
                break;
            }

            //--------------Write Hit-------------------------------------
            if( mem[set_no][i] == addr_bin.substr((int)log2(CACHE/ASSOC),TAG) ){
                //------Block Not Modified yet------
                    dirty[set_no][i]= 1;
                    
                    int write_lru_count=LRU_count[set_no][i];
                    //-------LRU Update------------
                    for (int j=0; j<ASSOC; j++){
                        if(LRU_count[set_no][j] < write_lru_count)
                            LRU_count[set_no][j] +=1;
                        }
                        LRU_count[set_no][i]= 0;
                    break;
            }
            //--------------Write Miss-------------------------------------
            else{
                //-----Write Allocate--------
                for(int j=0; j<ASSOC; j++){
                    //Replace The Lease Recently Used
                    if(LRU_count[set_no][j] == ASSOC-1){
                        dirty[set_no][j]=0;
                        mem[set_no][j] = addr_bin.substr((int)log2(CACHE/ASSOC),TAG);
                    }
                }                
                L1_write_miss+=1;
                write_back_signal=1;
                int write_lru_count=LRU_count[set_no][i];
                //-------LRU Update------------
                for (int j=0; j<ASSOC; j++){
                    if(LRU_count[set_no][j] < write_lru_count)
                    LRU_count[set_no][j] +=1;
                }
                LRU_count[set_no][i]= 0;
                break;
            
            }
            }
            
        }
    }

//-------------------------------------------------------------------------------------------------------------------------------------------------
//Print Cache Data
void print_data(){
        //cout<< "address in binary: "<< addr_bin<<endl;
        //cout<< "Set Number: "<< set_no<<endl;
        //Print Cache Tags
        // for(int i=0;i<(CACHE/(ASSOC*BLOCKSIZE));i++){
        // for(int j=0; j<ASSOC;j++){
        //     if(mem[i][j]!="-1")
        //     {   for(int k=TAG;k>=0;k--)
        //         cout<<mem[i][j][k];
        //     }
        //     else{
        //         cout<<mem[i][j];
        //     }
        //     cout<<"  |  ";
        // }
        //cout<<endl;
        // for(int j=0; j<ASSOC;j++){
        //     cout<<LRU_count[i][j]<<"   |  ";
        // }
        // cout<<endl;
        // }
        cout<<"Address: "<<addr_hex<<endl;
        cout<<"L1 read miss: "<<L1_read_miss<<endl;
        cout<<"L1 read: "<<L1_reads<<endl;
        cout<<"Call from next Level: "<<read_next_level<<endl;
}
};

unsigned int bin_to_dec(string bin, int start, int end){
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
string dec_to_bin(unsigned int addr_dec){
    unsigned int n=addr_dec;
    int i=0;
    string addr_bin="";
    while(i<32){
        string ch=to_string(n%2);
        addr_bin.append((string)(ch));
        n/=2;
        i++;
    }
    return addr_bin;
}

// Convert a hexadecimal number in string format to unsigned integer
int hex_to_dec(string str_addr){    
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

//--------------------------------------------------MAIN------------------------------------------------------------------
int main(){
    L1 sim;
    // string a="a0";
    
    // unsigned int addr= sim.hex_to_dec("dead0f00");
    // cout<<addr<<endl;
    // char bin[32];
    // sim.dec_to_bin(addr);
    // sim.print_addr();

    // string t="10101";
    // unsigned int td=sim.bin_to_dec(t);
    sim.get_addr('r',"adedebef");
    sim.print_data();
    sim.get_addr('r',"adede1ef");
    sim.print_data();
    sim.get_addr('r',"adede1ef");
    sim.print_data();
    sim.get_addr('r',"adedebef");
    sim.print_data();
    sim.get_addr('r',"adedefef");
    sim.print_data();
    return 0;
}