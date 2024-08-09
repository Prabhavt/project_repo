

# Prabhav Talukdar EE23M053

	This repo consists of all project files and reports done during my masters at IIT Madras.
	
	## Tiled Matrix Multiplication:
		Performing tiled matrix multiplication on a GPU using CUDA accelerated algorithm and
		compare result with CPU implementation, using basic profiling.
		
		Code File:
			1. matMul_cpu.c		// C Code for CPU Implementation
			2. matMul_gpu.cu	// CUDA code for GPU
		Profiling Result:
```
		==13787== NVPROF is profiling process 13787, command: ./test
		CPU Calculation Elapsed Time: 0.000000==13787== Profiling application: ./test
		==13787== Profiling result:
					Type  Time(%)      Time   Calls   Avg       Min       Max  Name
		GPU activities:   94.43%  314.31us     1  314.31us  314.31us  314.31us  mat_gpu(int*, int*, int*)
							3.12%  10.368us    2  5.1840us  5.1840us  5.1840us  [CUDA memcpy HtoD]
							2.45%  8.1610us    1  8.1610us  8.1610us  8.1610us  [CUDA memcpy DtoH]
```

		