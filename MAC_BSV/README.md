
# Multiply and Accumulate Module

Description:

MAC operation: A*B + C
Designing a Multiply-Accumulate (MAC) module that supports the MAC

operations for the following data types.

● S1. (A: int8 , B: int8 , C: int32)-> (MAC: int32)

● S2. (A: bf16, B: bf16, C: fp32)-> (MAC: fp32)


## Instructions to run:

Note: Uncomment the testbench lines in .bsv file if not already done.

On shell:
~$: build.sh <file name> mkTb <output filename > verilator

The program file names are:
    1. BF16MAC.bsv 
    2. int8_mac.bsv 
    3. unpipelined_BF16MAC.bsv 
    4. tunpipelined_int8mac.bsv

## To test your own inputs
On testbench folder replace the data of 

1. For int8 MAC:
    A_binaryint8.txt, B_binaryint8.txt
    C_binaryint8.txt

2. For BF16 MAC:
    A_bf16binary.txt, B_bf16binary.txt
    C_bf16binary.txt
