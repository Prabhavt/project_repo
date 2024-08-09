

# Tiled Matrix Multiplication

	Performing tiled matrix multiplication on a GPU using CUDA accelerated algorithm and
	compare result with CPU implementation, using basic profiling.
	
	Code File:
		1. matMul_cpu.c		// C Code for CPU Implementation
		2. matMul_gpu.cu	// CUDA code for GPU
	
	nvprof result:
	```
	CPU Calculation Elapsed Time: 0.000000==13787== Profiling application: ./test
	==13787== Profiling result:
            Type  Time(%)      Time     Calls       Avg       Min       Max  Name
 	GPU activities:   94.43%  314.31us         1  314.31us  314.31us  314.31us  mat_gpu(int*, int*, int*)
                   	  3.12%  10.368us         2  5.1840us  5.1840us  5.1840us  [CUDA memcpy HtoD]
                     	  2.45%  8.1610us         1  8.1610us  8.1610us  8.1610us  [CUDA memcpy DtoH]
      	API calls:   	  98.96%  72.543ms         3  24.181ms  2.4760us  72.535ms  cudaMalloc
                          0.52%  380.86us         3  126.95us  19.705us  339.36us  cudaMemcpy
                    	  0.25%  185.43us       101  1.8350us     144ns  71.709us  cuDeviceGetAttribute

	...continue
	```			
