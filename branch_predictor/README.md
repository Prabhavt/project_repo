# Branch Predictor Simulator

A C/C++ simulator for evaluating the performance of bimodal and gshare branch prediction strategies on benchmark trace files, developed as part of the CS6600: Computer Architecture course at IIT Madras.

📚 Project Overview

This simulator models and evaluates two types of branch predictors:

    Bimodal Predictor (a special case of gshare with no history)
    Gshare Predictor (uses global branch history and XOR indexing)

It processes trace files that represent the execution of benchmarks like gcc and jpeg, and reports prediction statistics including total branches, number of mispredictions, and misprediction rate.

[📊 Meta Data ](https://docs.google.com/spreadsheets/d/1zguzKgTKjlcnZFhuEQ20nFiDrAo30Dx8sTw6feJcwwM/edit?usp=sharing)

## 2 bit branch predictor
 - A 2-bit branch predictor is a dynamic branch prediction mechanism used in 
    modern processors to improve instruction pipeline flow by predicting the outcome of
    conditional branches. It uses a 2-bit saturating counter as a state machine 
    with four states: strongly not taken, weakly not taken, weakly taken, and strongly 
    taken. The predictor changes its prediction only after two consecutive mispredictions, 
    making it more robust against occasional anomalies compared to a 1-bit 
    predictor, which  flips its prediction after just one misprediction

<p align="center">
  <img src="https://github.com/user-attachments/assets/2f125386-dfe7-4d2d-8ce0-412499cf2e1f" alt="2_bit_predictor">
</p>

# Specification of Simulator
    Model a gshare branch predictor with parameters {m, n}, where:
    • m is the number of low-order PC bits used to form the prediction table index. Ensure to
    discard the lowest two bits of the PC, since these are always zero, i.e., use only bits m+1
    through 2 of the PC.
    • n is the number of bits in the global branch history register. Note:
        o n is typically less than or equal to m, i.e., n <= m.
        o n may be equal to zero, in which case we have the simple bimodal branch
          predictor.
## a. Bimodal Branch predictor
    When n=0, the gshare predictor reduces to a simple bimodal predictor. In this case, the index is
    based on only the branch’s PC, as illustrated in Fig. 1 below.
<p align="center">
  <img src="https://github.com/user-attachments/assets/ca921ffc-acef-4566-8de8-ac44c2f2c71c">
</p>

<p align="center">
  <img src="https://github.com/user-attachments/assets/9a06e6ee-6e97-453f-abac-a759ad588294">
</p>

 ❓ Discuss the trends (change in misprediction rate with increasing “m”) in each plot and describe
    the similarities/ differences between benchmarks.

 - We can observe that miss prediction reduces drastically with m for gcc trace, this means that there
   are more unpredictable branch pattern in the trace
 - For jpeg trace the decrease is less significant, which makes the branch pattern much predictable. 

 ❓ For each benchmark/trace, propose a bimodal predictor design that minimizes misprediction rate
    and predictor cost (in terms of storage requirement of the predictor table). Assume that you have
    a maximum budget of 16kB of storage for the predictor table.

- For gcc trace bimodal branch predictor, m= 12 gives us lowest miss prediction
and with a budget of 16KB. Our choice of m= 12 comes under the limit.
Size of predictor table will be 2 <sup>(12+1)</sup> = 1KB

- For jpeg trace bimodal branch predictor, we can observe that decline in miss prediction
  with m is minimal, hence we can have m=9. This reduces our prediction table size significantly
  Size of predictor table will be 2 <sup>(9+1)</sup> = 128B

## b. gShare Branch Predictor

    When n>0, we have the gshare branch predictor with n-bit global branch history register. In this
    case, the index is based on both the branch’s PC and the global branch history register, as shown
    in Fig. 2 below. The global branch history register should be initialized to all zeroes (00…0) and
    all entries in the prediction table should be initialized to 2 (“weakly taken”) at the beginning of
    the simulation.
    
<p align="center">
  <img src="https://github.com/user-attachments/assets/fff4142e-5974-4453-8b41-3aa8ff5639ed">
</p>

<p align="center">
  <img src="https://github.com/user-attachments/assets/4c496d6d-6769-404f-8ce5-a9185785b6f6">
</p>




