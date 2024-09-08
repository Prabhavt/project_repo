// Author:  Prabhav Talukdar
// Cache Simulator : L1 

#include<iostream>
#include<math.h>
#include <string>
#include<fstream>
#include<stdlib.h>

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


//--------------------------------------------Function Definations-------------------------------------------------------------
unsigned int bin_to_dec(string bin, int start, int end);
string dec_to_bin(unsigned int addr_dec);
int hex_to_dec(string str_addr);
string bin_to_hex(string addr_bin);

//--------------------------------------------Cache Class----------------------------------------------------------------------

class L1{
    string mem[L1_CACHE/(L1_ASSOC*BLOCKSIZE)][L1_ASSOC];                        
    string __cachefulladdr__[L1_CACHE/(L1_ASSOC*BLOCKSIZE)][L1_ASSOC];
    int valid[L1_CACHE/(L1_ASSOC*BLOCKSIZE)][L1_ASSOC];
    int dirty[L1_CACHE/(L1_ASSOC*BLOCKSIZE)][L1_ASSOC];
    int LRU_count[L1_CACHE/(L1_ASSOC*BLOCKSIZE)][L1_ASSOC];
    unsigned int set_no,display_setno;
    unsigned int L1_read_miss;
    unsigned int L1_reads;
    unsigned int L1_write_miss;
    unsigned int L1_writes;

protected:
    string addr_bin;
    string addr_hex;
    char op;                        
    string writeback_address;
    unsigned int read_L2;           // L1 -> L2  or  L2 -> Memory
    unsigned int write_back_L2;     //signals
    unsigned int write_back;        //count of total write backs from L1
public:

//--------------------------Initializing Cache information----------------------------------------------------------------------------------------
    L1(){
        for(int i=0;i<(L1_CACHE/(L1_ASSOC*BLOCKSIZE));i++){
            for(int j=0; j<L1_ASSOC;j++){
            mem[i][j]="-1";
            valid[i][j]=0;              //valid=0 : Not Valid          valid=1 : Valid
            LRU_count[i][j]=L1_ASSOC-1;
            dirty[i][j]=0;              //dirty=0 : Not Modified       dirty=1 : Modified
            }
        }
        L1_read_miss=0;
        L1_reads=0;
        L1_writes=0;
        L1_write_miss=0;
        read_L2=0;
        write_back_L2=0;                     
    }

    void get_addr_l1(char operation, string address){
        addr_hex=address;
        op= operation;
        unsigned int add_dec= hex_to_dec(address);
        addr_bin=dec_to_bin(add_dec);
        unsigned int set_no=bin_to_dec( addr_bin, ((int)log2(BLOCKSIZE)) , ((int)log2(L1_CACHE/L1_ASSOC))-1  );
        display_setno=set_no;
        read_L2=0; write_back_L2=0; 
    //---------------------------Read Operation------------------------------------------------------------------------------------------------------

            if(op=='r'){
                
                L1_reads+=1;
                int empty_block=0;


                for(int i=0; i<L1_ASSOC; i++){
                    if(valid[set_no][i]==0)
                        empty_block=1;
                    break;                            
                }
                
                for(int i=0; i<L1_ASSOC; i++){

                    //----------------------Read Hit-----------------------------------------------
                    if( mem[set_no][i] == addr_bin.substr((int)log2(L1_CACHE/L1_ASSOC),L1_TAG) ){
                        //-------LRU Update------------
                        LRU_update(set_no, i); 
                        break; 
                    }
            
                    //------------------Read Miss-------------------
                    if(i==(L1_ASSOC-1)){
                        L1_read_miss+=1;
                        read_L2=1;                                                  // Signaling next level for read

                        if(empty_block==1){
                            empty_block=0;
                            //-----------------------------------Cold Miss------------------------------- 
                            //---------------------Bring Data to Cold Blocks from Next Level--------------
                            //------------------------Read Signal to next Level--------------------------- 
                            for(int j=0; j<L1_ASSOC; j++){
                                if(valid[set_no][j]==0){
                                    mem[set_no][j]= addr_bin.substr((int)log2(L1_CACHE/L1_ASSOC),L1_TAG);
                                    __cachefulladdr__[set_no][j]=address;

                                    valid[set_no][j]=1;
                                    dirty[set_no][j]=0;
                                    //-------LRU Update------------
                                    LRU_update(set_no, j); 
                                    break;
                                }
                            }
                        }  
                        else
                        {
                        for(int j=0; j<L1_ASSOC; j++){
                            if( LRU_count[set_no][j] == (L1_ASSOC-1) ){                         
                                if(dirty[set_no][j]==1){
                                    write_back_L2=1;
                                    write_back++;
                                    writeback_address = __cachefulladdr__[set_no][j];                             
                                }
                                
                                mem[set_no][j] = addr_bin.substr((int)log2(L1_CACHE/L1_ASSOC),L1_TAG);
                                __cachefulladdr__[set_no][j]=address;

                                valid[set_no][j]= 1;
                                dirty[set_no][j]= 0;
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
                L1_writes+=1;
                int empty_block=0;
                
                //---Checking Line capacity---
                for(int i=0; i<L1_ASSOC; i++){              
                if(valid[set_no][i]==0)
                    empty_block=1;
                    break;                            
                }

                for(int i=0; i<L1_ASSOC; i++){

                //--------------Write Hit-------------------------------------
                if( mem[set_no][i] == addr_bin.substr((int)log2(L1_CACHE/L1_ASSOC),L1_TAG) ){
                        //------Dirty Bit Set----------
                        dirty[set_no][i]= 1;
                        valid[set_no][i]= 1;        
                        //-------LRU Update------------
                            LRU_update(set_no, i); 
                        break;
                }

                //--------------Write Miss and Write Back-------------------------------------
                if( (mem[set_no][i] != addr_bin.substr((int)log2(L1_CACHE/L1_ASSOC),L1_TAG)) && (i==(L1_ASSOC-1)) )
                {
                    L1_write_miss+=1;
                    read_L2=1;

                    //-----No Write Allocate if Invalid Block Present--------
                    if(empty_block==1){
                        empty_block=0;
                        for(int j=0; j<L1_ASSOC; j++){
                            if(valid[set_no][j]==0){
                                
                                mem[set_no][j] = addr_bin.substr((int)log2(L1_CACHE/L1_ASSOC),L1_TAG);
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
                        for(int j=0; j<L1_ASSOC; j++){
                            //Replace The Lease Recently Used Dirty Block
                            if(LRU_count[set_no][j] == L1_ASSOC-1){
                                if(dirty[set_no][j]==1){
                                    //Write allocate and Write in L1_CACHE
                                    write_back++;
                                    write_back_L2=1;
                                    writeback_address = __cachefulladdr__[set_no][j];                                                               
                                }
                                dirty[set_no][j]=1; 
                                mem[set_no][j] = addr_bin.substr((int)log2(L1_CACHE/L1_ASSOC),L1_TAG);
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

void LRU_update(int set, int pos){
    int write_lru_count=LRU_count[set][pos];
    //-------LRU Update------------
    for (int k=0; k<L1_ASSOC; k++){
        if(LRU_count[set][k] < write_lru_count)
        LRU_count[set][k] +=1;
    }
    LRU_count[set][pos]= 0;
}

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

//Print L1_CACHE Data
void print_data(){
        cout<<endl<<endl;
        cout<<"Configuration: "<<endl;
        cout<<"--------------Cache Simulator-----------------"<<endl;
        cout<<"----------Author: Prabhav Talukdar------------"<<endl;
        cout<<"----------------------------------------------"<<endl;
        for(int i=0;i<(L1_CACHE/(L1_ASSOC*BLOCKSIZE));i++){
            printf("set no. %3d %4.4s",i,"  | ");
        for(int j=0; j<L1_ASSOC;j++){
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
        cout<<"L1 read: "<<L1_reads<<endl;
        cout<<"L1 read miss: "<<L1_read_miss<<endl;
        cout<<"L1 write: "<<L1_writes<<endl;
        cout<<"L1 write miss: "<<L1_write_miss<<endl;
        cout<<"Write backs from L1: "<<write_back<<endl;
}
};



//-------------------------------------------------------------------------------------------------------------------------------------------------

class L2 {
    string mem[L2_CACHE/(L2_ASSOC*BLOCKSIZE)][L2_ASSOC];
    int valid[L2_CACHE/(L2_ASSOC*BLOCKSIZE)][L2_ASSOC];
    int dirty[L2_CACHE/(L2_ASSOC*BLOCKSIZE)][L2_ASSOC];
    int LRU_count[L2_CACHE/(L2_ASSOC*BLOCKSIZE)][L2_ASSOC];
    unsigned int set_no,display_setno;
    unsigned int L2_read_miss;
    unsigned int L2_reads;
    unsigned int L2_write_miss;
    unsigned int L2_writes;
    char op;
    string addr_bin;
    string addr_hex;

protected:
    unsigned int read_next_level;           // L1 -> L2  or  L2 -> Memory
    unsigned int write_back_signal;

public:

//--------------------------Initializing L2_CACHE information----------------------------------------------------------------------------------------
    L2(){
        for(int i=0;i<(L2_CACHE/(L2_ASSOC*BLOCKSIZE));i++){
            for(int j=0; j<L2_ASSOC;j++){
            mem[i][j]="-1";
            valid[i][j]=0;              //valid=0 : Not Valid          valid=1 : Valid
            LRU_count[i][j]=L2_ASSOC-1;
            dirty[i][j]=0;              //dirty=0 : Not Modified       dirty=1 : Modified
            }
        }
        L2_read_miss=0;
        L2_reads=0;
        L2_writes=0;
        L2_write_miss=0;
        read_next_level=0;
    }

    void get_addr_l2(char nxtlvl_op, string addr){
        op= nxtlvl_op;
        addr_hex=addr;
        unsigned int add_dec= hex_to_dec(addr_hex);
        addr_bin=dec_to_bin(add_dec);
        unsigned int set_no=bin_to_dec( addr_bin, ((int)log2(BLOCKSIZE)) , ((int)log2(L2_CACHE/L2_ASSOC))-1  );
        display_setno=set_no;
        read_next_level=0;
    //---------------------------Read Operation------------------------------------------------------------------------------------------------------

            if(op=='r'){
            
                L2_reads+=1;
                int empty_block=0;
                
                for(int i=0; i<L2_ASSOC; i++){
                    if(valid[set_no][i]==0)
                        empty_block=1;
                    break;                            
                }
                
                for(int i=0; i<L2_ASSOC; i++){

                    //----------------------Read Hit-----------------------------------------------
                    if( mem[set_no][i] == addr_bin.substr((int)log2(L2_CACHE/L2_ASSOC),L2_TAG) ){
                        //-------LRU Update------------
                        LRU_update(set_no, i); 
                        break; 
                    }

                    //----------------------Read Miss-----------------------------------------------
                    if(i==(L2_ASSOC-1)){
                        L2_read_miss+=1;
                        read_next_level=1;                // Signaling next level for read

                        if(empty_block==1){
                            empty_block=0;
                            // Cold Miss 
                            //---------------------Bring Data to Cold Blocks from Next Level-------------- 
                            for(int j=0; j<L2_ASSOC; j++){
                                if(valid[set_no][j]==0){
                                    valid[set_no][j]=1;
                                    dirty[set_no][j]=0;
                                    mem[set_no][j]= addr_bin.substr((int)log2(L2_CACHE/L2_ASSOC),L2_TAG);
                                    
                                    //-------LRU Update------------
                                    LRU_update(set_no, j); 
                                    break;
                                }
                            }
                        }  
                        else
                        {
                        for(int j=0; j<L2_ASSOC; j++){
                            if( LRU_count[set_no][j] == (L2_ASSOC-1) ){                         
                                if(dirty[set_no][j]==1){
                                    write_back_signal=1;                                  
                                }
                                
                                mem[set_no][j] = addr_bin.substr((int)log2(L2_CACHE/L2_ASSOC),L2_TAG);

                                valid[set_no][j]= 1;
                                dirty[set_no][j]= 0;
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

            else if(op=='w'){
                L2_writes+=1;
                write_back_signal=0;
                int empty_block=0;

                for(int i=0; i<L2_ASSOC; i++){
                if(valid[set_no][i]==0)
                    empty_block=1;
                    break;                            
                }

                for(int i=0; i<L2_ASSOC; i++){
                //--------------Write Hit-------------------------------------
                if( mem[set_no][i] == addr_bin.substr((int)log2(L2_CACHE/L2_ASSOC),L2_TAG) ){
                        //------Dirty Bit Set----------
                        dirty[set_no][i]= 1;        
                        valid[set_no][i]= 1; 
                        //-------LRU Update------------
                            LRU_update(set_no, i); 
                        break;
                }

                //--------------Write Miss and Write Back-------------------------------------
                if( (mem[set_no][i] != addr_bin.substr((int)log2(L2_CACHE/L2_ASSOC),L2_TAG)) && (i==(L2_ASSOC-1)) )
                {
                    L2_write_miss+=1;
                    cout<<addr<<" ";
                    //-----No Write Allocate if Invalid Block Present--------
                    if(empty_block==1){
                        empty_block=0;
                        for(int j=0; j<L2_ASSOC; j++){
                            if(valid[set_no][j]==0){
                                write_back_signal=1;
                                valid[set_no][j]=1;
                                dirty[set_no][j]=1;                
                                mem[set_no][j] = addr_bin.substr((int)log2(L2_CACHE/L2_ASSOC),L2_TAG);
    
                                //-------LRU Update------------
                                LRU_update(set_no, j); 
                                break;
                            }
                        }
                    //--------------------Write Allocate--------------------------               
                    }else{
                        for(int j=0; j<L2_ASSOC; j++){
                            //Replace The Lease Recently Used Dirty Block
                            if(LRU_count[set_no][j] == L2_ASSOC-1){
                                if(dirty[set_no][j]==1){
                                    //Write allocate and Write in L2_CACHE  
                                    write_back_signal=1;       
                                }
                                mem[set_no][j] = addr_bin.substr((int)log2(L2_CACHE/L2_ASSOC),L2_TAG);
                                
                                valid[set_no][j]=1;
                                dirty[set_no][j]= 1; 
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

void LRU_update(int set, int pos){
    int write_lru_count=LRU_count[set][pos];
    //-------LRU Update------------
    for (int k=0; k<L2_ASSOC; k++){
        if(LRU_count[set][k] < write_lru_count)
        LRU_count[set][k] +=1;
    }
    LRU_count[set][pos]= 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------
//Print L2_CACHE Data
void print_data(){
        cout<<endl<<endl;
        cout<<"Configuration: "<<endl;
        cout<<"--------------Cache Simulator-----------------"<<endl;
        cout<<"----------Author: Prabhav Talukdar------------"<<endl;
        cout<<"----------------------------------------------"<<endl;
        for(int i=0;i<(L2_CACHE/(L2_ASSOC*BLOCKSIZE));i++){
            printf("set no. %3d %4.4s",i,"  | ");
        for(int j=0; j<L2_ASSOC;j++){
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
        cout<<"L2 read: "<<L2_reads<<endl;
        cout<<"L2 read miss: "<<L2_read_miss<<endl;
        cout<<"L2 write: "<<L2_writes<<endl;
        cout<<"L2 write miss: "<<L2_write_miss<<endl;
}
};
//----------------------------------------------------------------------------------------------------------------------------


string bin_to_hex(string addr_bin){
    while(addr_bin.length()%4 != 0){
        addr_bin.append("0");
    }
    int dec=0;
    string hex_addr="";
    string digit="";
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
    int len=hex_addr.length();
    hex_addr= (hex_addr[len-1]=='0')? hex_addr.substr(0,len-1) : hex_addr;
    return hex_addr;
}
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
    string filepath="/home/prabhav/LocalDisk/EE23M053/CS6600 COA/Project/project1_cachesim/Assignment1/Assignment_files/gcc_trace.txt";
    ifstream file(filepath);
    string line;
    char op;
    char address[8];
    L1 sim_l1;
    L2 sim_l2;

    if (!file.is_open()) {
        cerr << "Error opening file." << endl;
        return -1;
    }
    
    int count=0;

    while (getline(file, line) ) {
        sscanf(line.c_str(), "%c %s", &op, address);
        string add=address;             // Converting character array to String 
        
        sim_l1.get_addr_l1(op,add);

        if(sim_l1.getSignal()==1)                           //Read
        {   
            sim_l2.get_addr_l2('r',add);
        }
        if(sim_l1.getSignal()==2){                          //First Write Back and then Read
            sim_l2.get_addr_l2('w',sim_l1.L2_write_addr());
            sim_l2.get_addr_l2('r',add);
        }
        //count++;
    }
    //cout<<writeback_addr("1101",18,5);
    sim_l1.print_data();
    sim_l2.print_data();

    file.close();
    return 0;
}