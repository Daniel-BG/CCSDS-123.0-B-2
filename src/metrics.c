#include "metrics.h"

double mse(int * data1, int * data2, int length) {
	double acc = 0;
	for (int i = 0; i < length; i++) {
		acc += (data1[i] - data2[i])*(data1[i] - data2[i]);
	}
	return acc / (double) length;
}