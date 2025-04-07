#ifndef BP_DEFINES
#define BP_DEFINES

#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <fstream>

#define ADDR_SIZE 32
#define M 4
#define N 2
#define SIZE_BT (1<<M)

typedef enum{
    NOT_TAKEN,
    WEAKLY_NOT_TAKEN,
    WEAKLY_TAKEN,
    TAKEN,
} states;

class bTableEntry{
    public:
    int index;
    states pred;
    bTableEntry(int _index, states _pred);
};

class bTable{
    public:
    int misprediction, total_instr;
    int BHR;
    std::list <bTableEntry> entries;
    bTable();

    // Get prediction value from table and if prediction is false, set it to outcome value
    states predict_bimodal(int _index, char outcome);

    states predict_gShare(int _index, char outcome);

    void print_bTable();
};

#endif
