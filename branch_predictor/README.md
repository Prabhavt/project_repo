# Branch Predictor Simulator

A C/C++ simulator for evaluating the performance of bimodal and gshare branch prediction strategies on benchmark trace files, developed as part of the CS6600: Computer Architecture course at IIT Madras.

📚 Project Overview

This simulator models and evaluates two types of branch predictors:

    Bimodal Predictor (a special case of gshare with no history)
    Gshare Predictor (uses global branch history and XOR indexing)

It processes trace files that represent the execution of benchmarks like gcc and jpeg, and reports prediction statistics including total branches, number of mispredictions, and misprediction rate.
