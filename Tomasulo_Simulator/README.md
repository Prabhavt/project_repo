# Simulator for an out-of-order superscalar processor based on Tomasulo’s algorithm
In this project, I will construct a simulator for an out-of-order superscalar processor based on
Tomasulo’s algorithm that fetches, dispatches, and issues N instructions per
cycle. The primary goal is to only model the dynamic scheduling mechanism in detail. Hence,
perfect caches and perfect branch prediction are assumed

## i. Project Overview

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

## ii. Outputs from Simulator
The simulator outputs the following measurements after completion of the run:
1. Total number of instructions in the trace.
2. Total number of cycles to finish the program.
3. Average number of instructions completed per cycle (IPC).
In addition, the simulator should also print the timing information for every instruction in the
trace.

## iii. Superscalar Microarchitecture
<p align="center">
  <img src="https://github.com/user-attachments/assets/0448de0c-677a-4122-9992-16b54aaa2865">
</p>

## iv. Build Command
```
  ubuntu@ubuntu:Tomasulo_Simulator$ ./build.sh N S <trace_name>
  <trace_name> : gcc / perl
```

## v. Experiment
Plots:
For each trace, plot IPC on the y axis and Scheduling Queue size (S) on the x
axis for S = 8, 16, 32, 64, 128, and 256. Produce a curve each for N = 2, 4, and 8.

<p align="center">
  <img src="https://github.com/user-attachments/assets/d0bb6ba6-7ec2-4c00-aea5-fb819e67a50d">
</p>

[📊 Data ](https://docs.google.com/spreadsheets/d/1VQENTrEkNfC-7Q20fedILziomy_0qKcAu0oyt3VvJcQ/edit?usp=sharing)

## vi. Observation
  - As Scheduling Queue Size (S) increases, IPC also increases <br>
      This is because with larger Scheduling Queue (S) it allows us to buffer more dependent instruction and allows all
      independent instructions to run. However if we keep increasing size of Scheduling queue (S), improvement in IPC
      becomes stagnant as we saturate all available Functional Units (FUs).
  - As Bandwidth Increases (N), IPC increases <br>
      This is because more instructions can be fetched and gives better flexibility to run more instructions.
  - Interaction of Scheduling Queue Size (S) and Bandwidth (N)
      - If we keep increasing Scheduling Queue Size (S) without increasing bandwidth, we wont have much of a benifit
       As we wont be able to fetch more instruction per cycle and fully utilize Scheduling Queue Size.
      - Similarly if we increase Bandwidth (N) and donot increase Scheduling Queue Size(S) proportionately, we will not have
        space to buffer incoming instruction. Hence a bottleneck.
