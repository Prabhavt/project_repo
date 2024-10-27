#! /usr/bin/bash

if [[ $# -eq 4 ]]
then
	if [[ $4 == "bsc" ]] 
	then 
		cd $PWD; mkdir ./intermediate; rm -r ./intermediate/*
		bsc --sim -u -g $2 $1
		bsc --sim --keep-fires -e $2 -o $3
		mv *.so *.o *.cxx *.h *.bo *.ba $3 ./intermediate
		./intermediate/sim
	elif [[ $4 == "verilator" ]]
	then
		cd $PWD;  
		if [[ ( ! -d "runs" ) ]]; then mkdir ./runs; fi;
	   	rm -r ./runs/*;
	   	cp ./testbench/* ./runs/
	   	cp ./src/$1 ./runs/
	   	cd ./runs/
	   	if [[ (! -d ./verilog ) ]]; then mkdir ./intermediate; mkdir ./verilog; fi 
		bsc -q -verilog -elab -vdir ./verilog -bdir ./intermediate -info-dir ./intermediate -g $2 $1
	        bsc -q -vsim verilator -vsearch ./verilog -fdir ./intermediate -e $2 -o $3
	        mv $3 ./intermediate/
		#mv -f *.so *.o *.cxx *.h *.bo *.ba $3 ./intermediate > /dev/null; true
		./intermediate/sim	
	else
		echo "Enter correct simulator"
	fi
elif [[ $# -eq 2 ]]
then
	if [[ $1 == "clean" ]]
	then 
		cd $PWD; 
		rm -r *.o *.cxx *.h $2 *.so *.bo *.ba
	fi  
else 
	echo -e "\nParameter to Script: <filename> <modulename> <output_file> <simulator(verilator/bsc)>"
	echo -e "\nProgram filename: \n\tBF16MAC.bsv \n\tint8_mac.bsv \n\tunpipelined_BF16MAC.bsv \n\tunpipelined_int8mac.bsv"
fi
