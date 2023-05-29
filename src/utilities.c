#include "utilities.h"

long min(long a, long b) {
	return a > b ? b : a;
}

long max(long a, long b) {
	return a > b ? a : b;
}

long clamp(long value, long min, long max) {
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

int clampi(int value, int min, int max) {
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}


long signum(long val) {
	if (val > 0)
		return 1;
	else if (val == 0)
		return 0;
	return -1;
}

int signum_plus(int val) {
	if (val >= 0)
		return 1;
	return -1;
}

long mod_R(long value, int r) {
	if (r == 64)
		return value;
	
	long offset = 1 << (r - 1);
	long modulus = 1 << r;
	return ((value + offset) % modulus) - offset;
}

long minus_one_to_the(long value) {
	if (value % 2 == 0)
		return 1;
	return -1;
}

