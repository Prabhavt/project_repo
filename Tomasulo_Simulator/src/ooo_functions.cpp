
#include "ooo_functions.h"

std::unordered_map<int,int> register_bank;
std::vector<INST> instruction_rob;
std::list<INST> fetch_list;
std::list<INST> dispatch_list;
std::list<INST> issue_list;
std::list<INST> execute_list;


INST::INST(unsigned long _pc, STATE _state, int _tag, int _op_type, int _rd, int _rs1, int _rs2)
{
    pc= _pc;
    st= _state;
    tag= _tag;
    op_type= _op_type;
    latency= _op_type == 0 ? 1 : ( _op_type == 1 ? 2 : 10 );
    rd= _rd;
    rs1= _rs1;
    rs2= _rs2;
    rs1_v=0;
    rs2_v=0;
    IF_t,IS_t,ID_t,EX_t,WB_t = std::make_pair(0,0); 
}

//-----------------------------------------------------RETIRE--------------------------------------------------------------------

void retire(){
    for(auto it = execute_list.begin(); it != execute_list.end(); ){
        INST temp = *it;
        if((temp.EX_t.first + temp.latency) == cc){
            temp.st= WB;
            temp.WB_t.first= cc;
            temp.WB_t.second= 1;
            if(temp.rd!=-1){
                for(auto i = issue_list.begin(); i != issue_list.end(); i++){
                    if(i->rs1_v == temp.tag) { i->rs1_v= 0; instruction_rob[temp.tag].rs1_v = 0; }
                    if(i->rs2_v == temp.tag) { i->rs2_v= 0; instruction_rob[temp.tag].rs2_v = 0; }
                }
                for(auto i = dispatch_list.begin(); i != dispatch_list.end(); i++){
                    if(i->rs1_v == temp.tag) { i->rs1_v= 0; }
                    if(i->rs2_v == temp.tag) { i->rs2_v= 0; }
                }
                if(register_bank[temp.rd] == temp.tag){ register_bank[temp.rd] =0;}
            }
            instruction_rob[temp.tag - 1]=temp;
            it= execute_list.erase(it);
        }else{
            it++;
        }
    }
}

//-----------------------------------------------------EXECUTE--------------------------------------------------------------------

void execute(){
        int issue_count=0;
        for (auto it = issue_list.begin() ; it != issue_list.end() && issue_count<N; ){
            INST temp = *it;
            if((!temp.rs1_v) && (!temp.rs2_v)){                 
                temp.st= EX;
                temp.IS_t.second= cc - temp.IS_t.first;         // Time calculation
                temp.EX_t.first = cc;
                temp.EX_t.second= temp.latency;

                execute_list.push_back(temp);
                instruction_rob[temp.tag - 1]= temp;
                
                issue_count++;
                it= issue_list.erase(it);                       //deletes element and puts it as next element
            }else{
                it++;
            }
        }
}

//-----------------------------------------------------ISSUE--------------------------------------------------------------------

void issue(){
    for(int i=0; i<N && issue_list.size()<S && !dispatch_list.empty(); i++){
        INST temp= dispatch_list.front();
        dispatch_list.pop_front();
        
        temp.st = IS;
        temp.ID_t.second= cc - temp.ID_t.first;         // Time calculation
        temp.IS_t.first = cc;

        issue_list.push_back(temp);
        instruction_rob[temp.tag - 1]=temp;
    }
}

//-----------------------------------------------------DISPATCH--------------------------------------------------------------------

void dispatch(){
    while((dispatch_list.size()<2*N) && !fetch_list.empty()){
        INST temp= fetch_list.front();
        fetch_list.pop_front();
        
        temp.st= ID;                                    // State Change
        temp.IF_t.second= cc - temp.IF_t.first;         // Time calculation
        temp.ID_t.first = cc;

        if(temp.rs1 != -1){ temp.rs1_v= register_bank[temp.rs1];}
        if(temp.rs2 != -1){ temp.rs2_v= register_bank[temp.rs2];}
        //I am using instruction tag as producer/destination register tag 
        if(temp.rd  != -1){
        register_bank[temp.rd] = temp.tag;                // register tag =-1 -> No operand
        }                                                 // register tag = 0 -> register available
                                                          // register tag > 0 -> holds producer tag

        dispatch_list.push_back(temp);
        instruction_rob[temp.tag - 1]= temp;
    }
}

//-----------------------------------------------------FETCH--------------------------------------------------------------------

void fetch(unsigned long pc, int tag, int op_type, int rd, int rs1, int rs2)
{   
    INST temp = INST(pc, IF, tag, op_type, rd, rs1, rs2);
    temp.IF_t.first= cc;

    no_instruction++; 
    instruction_rob.push_back(temp);
    fetch_list.push_back(temp);
}



//-----------------------------------------------------PRINT--------------------------------------------------------------------
void print_output(){
    for(int i=0; i< instruction_rob.size(); i++ ){
        std::cout<<i<<" ";
        std::cout<<"fu{"<<instruction_rob[i].op_type<<"} ";
        std::cout<<"src{"<<instruction_rob[i].rs1<<","<<instruction_rob[i].rs2<<"} ";
        std::cout<<"dst{"<<instruction_rob[i].rd<<"} ";
        std::cout<<"IF{"<<instruction_rob[i].IF_t.first<<","<<instruction_rob[i].IF_t.second<<"} ";
        std::cout<<"ID{"<<instruction_rob[i].ID_t.first<<","<<instruction_rob[i].ID_t.second<<"} ";
        std::cout<<"IS{"<<instruction_rob[i].IS_t.first<<","<<instruction_rob[i].IS_t.second<<"} ";
        std::cout<<"EX{"<<instruction_rob[i].EX_t.first<<","<<instruction_rob[i].EX_t.second<<"} ";
        std::cout<<"WB{"<<instruction_rob[i].WB_t.first<<","<<instruction_rob[i].WB_t.second<<"} \n";
    }
    std::cout<<"CONFIGURATION\n";
    std::cout<<" superscalar bandwidth (N)\t= "<<N<<std::endl;
    std::cout<<" dispatch queue size (2*N)\t= "<<2*N<<std::endl;
    std::cout<<" schedule queue size (S)\t= "<<S<<std::endl;
    std::cout<<"RESULTS"<<std::endl;
    std::cout<<" number of instructions\t\t= "<<no_instruction<<std::endl;
    std::cout<<" number of cycles\t\t= "<<(cc+1)<<std::endl;
    std::cout<<std::setprecision(4)<<" IPC\t\t\t\t= "<< (float)no_instruction/cc <<std::endl;
}

void print_debug(){
    std::cout<<"\n\nPrinting ROB to debug\n\n";
    std::cout<<"Index\tPC\tstate\top_type\tlatency\ttag\tIF\tID\tIS\tEX\tWB\n";
    int index=0;
    for(int i=0; i< instruction_rob.size(); i++ ){  
        std::cout<<index++<<"\t";
        std::cout<<std::hex<<instruction_rob[i].pc;
        std::cout<<std::dec<<"\t"<<instruction_rob[i].st<<"\t";
        std::cout<<instruction_rob[i].op_type<<"\t";
        std::cout<<instruction_rob[i].latency<<"\t"<<instruction_rob[i].tag<<"\t";
        std::cout<<instruction_rob[i].IF_t.first<<","<<instruction_rob[i].IF_t.second<<"\t";
        std::cout<<instruction_rob[i].ID_t.first<<","<<instruction_rob[i].ID_t.second<<"\t";
        std::cout<<instruction_rob[i].IS_t.first<<","<<instruction_rob[i].IS_t.second<<"\t";
        std::cout<<instruction_rob[i].EX_t.first<<","<<instruction_rob[i].EX_t.second<<"\t";
        std::cout<<instruction_rob[i].WB_t.first<<","<<instruction_rob[i].WB_t.second<<"\t\n";
    }
}