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
#define CACHE               1024      // Number of Set x Associativity x Block Offset
#define ASSOC               2         // Associativity
#define BLOCKSIZE           16        // Size in Bytes           
#define TAG                 32-(int)log2(CACHE/ASSOC)
using namespace std;


//--------------------------------------------Function Definations-------------------------------------------------------------
unsigned int bin_to_dec(string bin, int start, int end);
string dec_to_bin(unsigned int addr_dec);
int hex_to_dec(string str_addr);
string bin_to_hex(string addr_bin);

//--------------------------------------------Cache Class----------------------------------------------------------------------

class L1{
    string mem[CACHE/(ASSOC*BLOCKSIZE)][ASSOC];
    int valid[CACHE/(ASSOC*BLOCKSIZE)][ASSOC];
    int dirty[CACHE/(ASSOC*BLOCKSIZE)][ASSOC];
    int LRU_count[CACHE/(ASSOC*BLOCKSIZE)][ASSOC];
    unsigned int set_no,display_setno;
    unsigned int L1_read_miss;
    unsigned int L1_reads;
    unsigned int L1_write_miss;
    unsigned int L1_writes;

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
            valid[i][j]=0;              //valid=0 : Not Valid          valid=1 : Valid
            LRU_count[i][j]=ASSOC-1;
            dirty[i][j]=0;              //dirty=0 : Not Modified       dirty=1 : Modified
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
        display_setno=set_no;

    //---------------------------Read Operation------------------------------------------------------------------------------------------------------

            if(op=='r'){
                
                L1_reads+=1;
                read_next_level=0;
                int empty_block=0;
                
                for(int i=0; i<ASSOC; i++){
                    if(valid[set_no][i]==0)
                        empty_block=1;
                    break;                            
                }
                
                for(int i=0; i<ASSOC; i++){

                    //----------------------Read Hit-----------------------------------------------
                    if( mem[set_no][i] == addr_bin.substr((int)log2(CACHE/ASSOC),TAG) ){
                        //-------LRU Update------------
                        LRU_update(set_no, i); 
                        break; 
                    }

                    if(i==(ASSOC-1)){
                        L1_read_miss+=1;
                        read_next_level=1;                                                  // Signaling next level for read
                        //Read Miss
                        //Data brought in from next level of memory heirarchy 
                        if(empty_block==1){
                            empty_block=0;
                            // Cold Miss 
                            //---------------------Bring Data to Cold Blocks from Next Level-------------- 
                            for(int j=0; j<ASSOC; j++){
                                if(valid[set_no][j]==0){
                                    valid[set_no][j]=1;
                                    dirty[set_no][j]=0;
                                    mem[set_no][j]= addr_bin.substr((int)log2(CACHE/ASSOC),TAG);
                                    
                                    //-------LRU Update------------
                                    LRU_update(set_no, j); 
                                    break;
                                }
                            }
                        }  
                        else
                        {
                        for(int j=0; j<ASSOC; j++){
                            if( LRU_count[set_no][j] == (ASSOC-1) ){                         
                                if(dirty[set_no][j]==1){
                                    write_back_signal=1; 
                                    dirty[set_no][j]=0;                                  
                                }
                                mem[set_no][j] = addr_bin.substr((int)log2(CACHE/ASSOC),TAG);
                                valid[set_no][j]= 1;
                        
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

            else{
                L1_writes+=1;
                write_back_signal=0;
                int empty_block=0;

                for(int i=0; i<ASSOC; i++){
                if(valid[set_no][i]==0)
                    empty_block=1;
                    break;                            
                }

                for(int i=0; i<ASSOC; i++){
                //--------------Write Hit-------------------------------------
                if( mem[set_no][i] == addr_bin.substr((int)log2(CACHE/ASSOC),TAG) ){
                        //------Dirty Bit Set----------
                        dirty[set_no][i]= 1;        
                        
                        //-------LRU Update------------
                            LRU_update(set_no, i); 
                        break;
                }

                //--------------Write Miss and Write Back-------------------------------------
                if( (mem[set_no][i] != addr_bin.substr((int)log2(CACHE/ASSOC),TAG)) && (i==(ASSOC-1)) )
                {
                    L1_write_miss+=1;
                    //-----No Write Allocate if Invalid Block Present--------
                    if(empty_block==1){
                        empty_block=0;
                        for(int j=0; j<ASSOC; j++){
                            if(valid[set_no][j]==0){
                                write_back_signal=1;
                                valid[set_no][j]=1;
                                dirty[set_no][j]=0;                 //Empty block allocated 
                                mem[set_no][j] = addr_bin.substr((int)log2(CACHE/ASSOC),TAG);
    
                                //-------LRU Update------------
                                LRU_update(set_no, j); 
                                break;
                            }
                        }
                    //--------------------Write Allocate--------------------------               
                    }else{
                        for(int j=0; j<ASSOC; j++){
                            //Replace The Lease Recently Used Dirty Block
                            if(LRU_count[set_no][j] == ASSOC-1){
                                if(dirty[set_no][j]==1){
                                    //Write allocate and Write in cache  
                                    write_back_signal=1;
                                    dirty[set_no][j]=1;                                  
                                }
                                mem[set_no][j] = addr_bin.substr((int)log2(CACHE/ASSOC),TAG);

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
    for (int k=0; k<ASSOC; k++){
        if(LRU_count[set][k] < write_lru_count)
        LRU_count[set][k] +=1;
    }
    LRU_count[set][pos]= 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------
//Print Cache Data
void print_data(){
        cout<<endl<<endl;
        cout<<"Configuration: "<<endl;
        cout<<"--------------Cache Simulator-----------------"<<endl;
        cout<<"----------Author: Prabhav Talukdar------------"<<endl;
        cout<<"----------------------------------------------"<<endl;
        for(int i=0;i<(CACHE/(ASSOC*BLOCKSIZE));i++){
            printf("set no. %2d %4.4s",i,"  | ");
        for(int j=0; j<ASSOC;j++){
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
}
};

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
    L1 sim;

    if (!file.is_open()) {
        cerr << "Error opening file." << endl;
        return -1;
    }

    while (getline(file, line)) {
        sscanf(line.c_str(), "%c %s", &op, address);
        string add=address;             // Converting character array to String 
        sim.get_addr(op,address);
    }

    sim.print_data();

    file.close();
    return 0;
}