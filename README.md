

# Prabhav Talukdar EE23M053

	This repository consists of all project files/reports done during my masters at IIT Madras.
	
	1. Tiled Matrix Multiplication:
		Performing tiled matrix multiplication on a GPU using CUDA accelerated algorithm and
		compare result with CPU implementation, using basic profiling.
		
		Code File:
			1. matMul_cpu.c		// C Code for CPU Implementation
			2. matMul_gpu.cu	// CUDA code for GPU
   
	2. Design a Cache and Memory hierarchy simulator
 		Implement a simulator to compare the performance, area, and energy of different 
		memory hierarchy configurations, using a subset of the SPEC-2000 benchmark suite.
		Implement L1, L2, victim cache with LRU replacement policy and different configs.
	
 	3. Asynchronous FIFO:
		Design a 16 byte deep Asynchronous FIFO using Verilog
		
		Code File:
			1. FIFO_top.v			//Top Module
			2. memory.v			//FIFO memory block of 16bytes
			3. ptr_module.v			//pointer module to handle read and write
			4. twoflopsynchronizer.v	//Two FF synchronizer to synch cross domain
							//signals

		
