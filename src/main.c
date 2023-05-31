#include "ccsds_1230b2_codec.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "checker.h"
#include "debug.h"



bool equal(int * data1, int * data2, int length) {
	for (int i = 0; i < length; i++) {
		if (data1[i] != data2[i]) {
			printf("Different @ %i", i);
			return false;
		}
	}
	return true;
}

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

int main() {
	if (sizeof(long) != 8) {
		printf("Long must be 8 bytes for this to work\n");
		return -1;
	}

	Checker * checker = create_checker();
	set_message(checker, "default");

	//initialize compression parameters
	int bands = 32, lines = 32, samples = 32;
	CompressionParameters * cp = calloc(1, sizeof(CompressionParameters));
	set_defaults(cp);
	set_dimensions(cp, bands, lines, samples);

	//initialize a cube of size 16
	int * block = calloc(cp->samples_per_image, sizeof(int));
	random_fill(block, cp->samples_per_image, time(NULL));
	//random_fill(block, cp->samples_per_image, 0); //fixed seed

	//compress
	BitOutputStream * bos = create_bos();
	compress(block, cp, bos, checker);

	//decompress
	BitInputStream * bis = create_bis(bos->data);
	free(bos);
	set_defaults(cp);
	set_dimensions(cp, bands, lines, samples);
	int * decoded_block = decompress(bis, cp, checker);

	//print result
	if (equal(block, decoded_block, cp->samples_per_image))
		printf("Success\n");
	else
		printf("Failure\n");
}