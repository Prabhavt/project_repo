#! /usr/bin/bash

if [[ ! -d "runs" ]] 
then 
    mkdir runs; 
 else 
    find runs -empty -delete
fi
cd $PWD/src/
g++ -w bpredictor.cpp  bpredictor_defines.h  bpredictor_functions.cpp -o ../runs/sim

# Building for Bimodal Predictor
if [[ ! -d ../runs/bimodal ]]; then mkdir -p ../runs/bimodal; fi
for (( i=7; i<13; i++ )) do
    if [[ ! -f ../runs/bimodal/out_gcc_trace_$i.txt || ! -f ../runs/bimodal/out_jpeg_trace_$i.txt ]]
    then
    (../runs/sim $i < ../testbench/gcc_trace.txt) > ../runs/bimodal/out_gcc_trace_$i.txt
    (../runs/sim $i < ../testbench/jpeg_trace.txt) > ../runs/bimodal/out_jpeg_trace_$i.txt
    fi
done

# Building for GShare Predictor
if [[ ! -d ../runs/gshare ]]; then mkdir -p ../runs/gshare; fi
for (( m=7; m<13; m++ )) do
	for (( n=2; n<=m; n+=2 )) do 
    		if [[ ! -f ../runs/gshare/out_gcc_trace_$m_$n.txt || ! -f ../runs/gshare/out_jpeg_trace_$m_$n.txt ]]
    		then
    		(../runs/sim $m $n < ../testbench/gcc_trace.txt) > ../runs/gshare/out_gcc_trace_${m}_${n}.txt
    		(../runs/sim $m $n < ../testbench/jpeg_trace.txt) > ../runs/gshare/out_jpeg_trace_${m}_${n}.txt
    		fi
    done
done
