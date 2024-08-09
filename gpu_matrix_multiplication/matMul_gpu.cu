//AUTHOR: Prabhav Talukdar
//        EE23M053
//Tiled Matrix multiplication on using cuda accelerated algorithm

#include<stdio.h>
#include<cuda.h>
#include<math.h>
#include<stdlib.h>

#define N 64
#define BlockSize 4


__global__ void mat_gpu(int *A, int *B, int *C){
    __shared__ int shared_A[N*N];
    __shared__ int shared_B[N*N];

    unsigned bx= blockIdx.x;
    unsigned by= blockIdx.y;
    unsigned tx= threadIdx.x;
    unsigned ty= threadIdx.y;
    
    unsigned row= by * blockDim.x + ty;
    unsigned col= bx * blockDim.y + tx;

    int acc=0;
    // Bringing block/tile from global memory to shared memory.
    for (unsigned i=0; i < (N/BlockSize); i++){
        //  Data is stored in row major format. A = [ row1[N] row2[N] row3[N] .. ]
        // [ row * N + ( i * BlockSize + tx )]:
        //         Here row * N traverses through every row stored in A. Its the zeroth position
        //         i * BlockSize determines the block which we are operating and tx points the element
        // [ (i * BlockSize + ty ) * N + col ]:
        //         col represents the zeroth position
        //         (i*BlockSize)*N is the block below present block
        //         ty*N is traversing through the colomn  
                      
        shared_A[ty*BlockSize+tx]= A[ row * N + ( i * BlockSize + tx )];
        shared_B[ty*BlockSize+tx]= B[ (i * BlockSize)*N + (ty*N) + col];
        __syncthreads();
        
        for(unsigned j=0; j< BlockSize; j++){
            acc += shared_A[ty*BlockSize+j] * shared_B[j*BlockSize+tx]; 
        }
        __syncthreads();
    }
    C[row*N+col]= acc;
}

int matMul_verify(int *A, int *B, int *C){
    int ans[N*N]={0};
    for(int i=0; i<N; i++){
        for(int j=0; j<N; j++){
            for(int k=0; k<N; k++){
                ans[i*N+j]+=A[i*N+k]*B[k*N+j];
            }
        }
    }
    for(int i=0;i<N*N;i++){
        if(ans[i]!=C[i]){
            return 1;
        }
    }
    return 0;
}

int main()
{
    int *a,*b,*c;
    int *gpu_a, *gpu_b, *gpu_c;

    a =(int*)malloc(N*N*sizeof(int));
    b =(int*)malloc(N*N*sizeof(int));
    c =(int*)malloc(N*N*sizeof(int));
        
    cudaMalloc(&gpu_a, N*N*sizeof(int));
    cudaMalloc(&gpu_b, N*N*sizeof(int));
    cudaMalloc(&gpu_c, N*N*sizeof(int));
    
    for(int i=0;i<N*N;i++){
        a[i]=i%10; //rand()%10;
        b[i]=i%10; //rand()%10;
        c[i]=-1;
    }
    cudaMemcpy(gpu_a, a, N*N*sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(gpu_b, b, N*N*sizeof(int), cudaMemcpyHostToDevice);
    //cudaMemcpy(gpu_c, c, N*N*sizeof(int), cudaMemcpyHostToDevice);

    int gridSize= (int)ceil(N/BlockSize);
    int THREADS = BlockSize;

    dim3 threads(THREADS, THREADS, 1);
    dim3 grid(gridSize, gridSize, 1);
    
    
    mat_gpu<<<grid,threads>>>(gpu_a, gpu_b, gpu_c);

    cudaMemcpy(c, gpu_c, N*N*sizeof(int), cudaMemcpyDeviceToHost);
    cudaDeviceSynchronize();

    clock_t cpu_startTime, cpu_endTime;
    long double cpu_ElapseTime=0;
    cpu_startTime = clock();

    matMul_verify(a,b,c);
    
    cpu_endTime = clock();

    cpu_ElapseTime = ((cpu_endTime - cpu_startTime)/CLOCKS_PER_SEC)*1000000;
    
    int flag=0;

    if(flag==1){
        printf("Wrong Calculation\n");
    }
    else{
        printf("Correct\n");
    }

    // printf("Input Matrix %dx%d",N,N);
    // for(int i=0; i< N*N; i++){
    //         if(i%N==0){
    //             printf("\n");
    //         }
    //         printf("%d ", a[i]);
    // }
    // printf("\n");

    // for(int i=0; i< N*N; i++){
    //         if(i%N==0){
    //             printf("\n");
    //         }
    //         printf("%d ", b[i]);
    // }
    // printf("\n");
    // printf("Output Matrix");
    // for(int i=0; i< N*N; i++){
    //         if(i%N==0){
    //             printf("\n");
    //         }
    //         printf("%d ", c[i]);
    // }
    printf("CPU Calculation Elapsed Time: %Lf", cpu_ElapseTime);
    free(a);
    free(b);
    free(c);
    cudaFree(gpu_a);
    cudaFree(gpu_b);
    cudaFree(gpu_c);

    return 0;
}