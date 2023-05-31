#include "utilities.h"

void random_fill(int * data, int length, long seed) {
	srand(seed);   // Initialization, should only be called once.
	int i;
	for (i = 0; i < length; i++)
		data[i] = rand() & 0xffff; 
}

void sequential_fill(int * data, int length) {
	for (int i = 0; i < length; i++) {
		data[i] = i;
	}
}



