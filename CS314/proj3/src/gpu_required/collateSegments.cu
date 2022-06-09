/*
 **********************************************
 *  CS314 Principles of Programming Languages *
 *  Spring 2020                               *
 **********************************************
 */
#include <stdio.h>
#include <stdlib.h>

__global__ void collateSegments_gpu(int * src, int * scanResult, int * output, int numEdges) {
	int tid = (blockIdx.x * blockDim.x) + threadIdx.x;
	int total_threads = blockDim.x * gridDim.x;
	for(int i = tid; i < numEdges; i+=total_threads) {
		if((i+1) < numEdges) {
			if(src[i+1] != src[i]) output[src[i]] = scanResult[i];
		}
		else output[src[i]] = scanResult[i];
	}
}
