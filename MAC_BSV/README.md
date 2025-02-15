
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
```bash
ubuntu@ubuntu:~$  ./build.sh  <file-name>  mkTb  <output-filename>  verilator

Example:
ubuntu@ubuntu:~$  ./build.sh BF16MAC.bsv mkTb sim verilator
```

The program file names are:
    
    1. BF16MAC.bsv                         Pipelined BF16 MAC Design Completed & Verified using Bluespec testbench
    2. int8_mac.bsv                        Pipelined int8 MAC Design Completed & Verified using Bluespec testbench
    3. unpipelined_BF16MAC.bsv             Unpipelined BF16MAC Design Completed & Verified using Bluespec testbench
    4. unpipelined_int8mac.bsv             Unpipelined int8 mac Design Completed & Verified using Bluespec testbench

## To test your own inputs
On testbench folder replace the data of 

    1. For int8 MAC:
        A_binaryint8.txt, B_binaryint8.txt
        C_binaryint8.txt

    2. For BF16 MAC:
        A_bf16binary.txt, B_bf16binary.txt
        C_bf16binary.txt


## BF16 MAC Architecture

   ![BF16_Mul](https://github.com/user-attachments/assets/25e9983c-99e7-4e7d-b41f-d803c07bc547)  
   ![BF16_Add](https://github.com/user-attachments/assets/ab6d8fb9-d316-41ea-bfa7-bcc456d866f5)

