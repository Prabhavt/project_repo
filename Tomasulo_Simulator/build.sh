#! /bin/bash

mkdir -p ${PWD}/runs
g++ ${PWD}/src/ooo_main.cpp ${PWD}/src/ooo_functions.cpp -o ${PWD}/runs/sim 

if [[ "$3" == "gcc" ]]
then
	${PWD}/runs/sim $1 $2 < ${PWD}/testbench/gcc_trace.txt > ${PWD}/runs/gcc_val.txt
	diff -iw $PWD/runs/gcc_val.txt ${PWD}/testbench/expected_outputs/gcc_val.txt

elif [[ "$3" == "perl" ]]
then
	${PWD}/runs/sim $1 $2 < ${PWD}/testbench/perl_trace.txt > ${PWD}/runs/perl_val.txt
	diff -iw $PWD/runs/perl_val.txt ${PWD}/testbench/expected_outputs/perl_val.txt
else
	echo -e " \t ./build N S <trace_name> \n \t <tace_name>: gcc or perl"
fi


