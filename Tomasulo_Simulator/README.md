# Simulator for an out-of-order superscalar processor based on Tomasulo’s algorithm
In this project, I will construct a simulator for an out-of-order superscalar processor based on
Tomasulo’s algorithm that fetches, dispatches, and issues N instructions per
cycle. The primary goal is to only model the dynamic scheduling mechanism in detail. Hence,
perfect caches and perfect branch prediction are assumed

## 📚 Project Overview

The simulator reads a trace file in the following format:
```
  <PC> <operation type> <dest reg#> <src1 reg#> <src2 reg#>
  <PC> <operation type> <dest reg#> <src1 reg#> <src2 reg#>
  …
```
Where:
<br>• **< PC >** is the program counter of the instruction, which is specified in hexadecimal.
<br>• **< operation type >** is either “0”, “1”, or “2”.
<br>• **< dest reg# >** is the destination register of the instruction. If it is -1, then the
instruction does not have a destination register. Otherwise, it is between 0 and 127.
<br>• **< src1 reg# >** is the first source register of the instruction. If it is -1, then the
instruction does not have a first source register. Otherwise, it is between 0 and 127.
<br>• **< src2 reg# >** is the second source register of the instruction. If it is -1, then the
instruction does not have a second source register. Otherwise, it is between 0 and 127.

Example trace:
<br> bc020064 0 1 2 3
<br> bc020068 1 4 1 3
<br> bc02006c 2 -1 4 7

Means:
<br> “operation type 0” R1, R2, R3
<br> “operation type 1” R4, R1, R3
<br> “operation type 2” -, R4, R7

## Outputs from Simulator
The simulator outputs the following measurements after completion of the run:
1. Total number of instructions in the trace.
2. Total number of cycles to finish the program.
3. Average number of instructions completed per cycle (IPC).
In addition, the simulator should also print the timing information for every instruction in the
trace.

## Superscalar Microarchitecture



<p align="center">
  <img src="">
</p>
