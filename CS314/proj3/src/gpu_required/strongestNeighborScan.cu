/*
 **********************************************
 *  CS314 Principles of Programming Languages *
 *  Spring 2020                               *
 **********************************************
 */
#include <stdio.h>
#include <stdlib.h>

__global__ void strongestNeighborScan_gpu(int * src, int * oldDst, int * newDst, int * oldWeight, int * newWeight, int * madeChanges, int distance, int numEdges) {
	int tid = (blockIdx.x * blockDim.x) + threadIdx.x;
	int total_threads = blockDim.x * gridDim.x;
	for(int i = tid; i < numEdges; i+=total_threads) {
		if(i-distance >= 0) {
			if(src[i] == src[i-distance]) {
				if(oldWeight[i] == oldWeight[i-distance]) {
					if(oldDst[i] < oldDst[i-distance]) {
						newWeight[i] = oldWeight[i];
						newDst[i] = oldDst[i];
					}
					else {
						newWeight[i] = oldWeight[i-distance];
						newDst[i] = oldDst[i-distance];
						madeChanges[0] = 1;
					}
				}
				else if(oldWeight[i] > oldWeight[i-distance]){
					newWeight[i] = oldWeight[i];
					newDst[i] = oldDst[i];
				}
				else {
					newWeight[i] = oldWeight[i-distance];
					newDst[i] = oldDst[i-distance];
					madeChanges[0] = 1;
				}
			}
			else {
				newWeight[i] = oldWeight[i];
				newDst[i] = oldDst[i];
			}
		}
		else {
			newWeight[i] = oldWeight[i];
			newDst[i] = oldDst[i];
		}
	}
}
