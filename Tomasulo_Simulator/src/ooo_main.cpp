#include "ooo_functions.h"
//-------------------------------------------------Main Function--------------------------------------------------
int             N,S;
int             cc =0;
int             no_instruction=0;

int main(int argc, char* argv[]){
    N=std::stoi(argv[1]), S=std::stoi(argv[2]);          
    
    register_bank[-1]  =0;
    unsigned long   pc =0;
    int             op_type, rd, rs1, rs2;    
    int             tag=1;
    

    do{
        //------------------------WRITE BACK-----------------------------
            retire();           

        //------------------------EXECUTE--------------------------------
            execute();             

        //------------------------ISSUE-------------------------------
            issue();

        //------------------------DISPATCH-------------------------------
            dispatch();   

        //------------------------FETCH----------------------------------
        for( int i= 0; ((i<N) && (dispatch_list.size()+fetch_list.size()< 2*N)) ; i++ ){
            std::cin >> std::hex >> pc; 
            std::cin >> std::dec >> op_type >> rd >> rs1 >> rs2;
            if(pc==-1){ break;}

            fetch(pc,tag,op_type,rd,rs1,rs2);
            tag++;
        }


        if(!fetch_list.size() && !issue_list.size() && !dispatch_list.size() && !execute_list.size()) break;
    }while(++cc);
    print_output();
    
    return 0;   
}