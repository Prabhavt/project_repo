#ifndef OOO_DEFINES
#define OOO_DEFINES

#include<iostream>
#include<vector>
#include<list>
#include<string>
#include <unordered_map>
#include <iomanip>

typedef enum{
    IF,
    ID,
    IS,
    EX,
    WB,
    END
} STATE;

extern int N, S, cc, no_instruction;

class INST{
    public:
    INST(unsigned long pc, STATE state, int tag, int op_type, int rd, int rs1, int rs2);
    unsigned long pc;
    int tag;
    STATE st;
    int op_type;
    int latency;
    int rd, rs1, rs2;           // Register Name, ex- R06
    int rs1_v, rs2_v;           //Tag or Value of Register
    std::pair<int,int> IF_t,IS_t,ID_t,EX_t,WB_t;
};

extern    std::unordered_map<int,int> register_bank;
extern    std::vector<INST> instruction_rob;
extern    std::list<INST> fetch_list;
extern    std::list<INST> dispatch_list;
extern    std::list<INST> issue_list;
extern    std::list<INST> execute_list;

void fetch(unsigned long pc, int tag, int op_type, int rd, int rs1, int rs2);
void dispatch();
void issue();
void execute();
void retire();

void print_output();
void print_debug();
#endif