#include "metrics.h"

double mse(long * data1, long * data2, long length) {
	double acc = 0;
	for (int i = 0; i < length; i++) {
		acc += (data1[i] - data2[i])*(data1[i] - data2[i]);
	}
	return acc / (double) length;
}