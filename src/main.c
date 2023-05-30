#include "ccsds_1230b2_codec.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "checker.h"


bool equal(int * data1, int * data2, int length) {
	for (int i = 0; i < length; i++) {
		if (data1[i] != data2[i]) {
			printf("Different @ %i", i);
			return false;
		}
	}
	return true;
}

void random_fill(int * data, int length) {
	srand(time(NULL));   // Initialization, should only be called once.
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

	int bands = 16, lines = 16, samples = 16;

	//initialize compression parameters
	CompressionParameters * cp = calloc(1, sizeof(CompressionParameters));
	set_defaults(cp);
	set_dimensions(cp, bands, lines, samples);

	//initialize a cube of size 16
	int * block = calloc(cp->samples_per_image, sizeof(int));
	sequential_fill(block, cp->samples_per_image);
	//random_fill(block, cp->samples_per_image);

	//try to compress
	BitOutputStream * bos = create_bos();
	compress(block, cp, bos, checker);

	//try to decompress
	BitInputStream * bis = create_bis(bos->data);
	free(bos);
	int * decoded_block = decompress(bis, cp, checker);

	//print result
	if (equal(block, decoded_block, cp->samples_per_image))
		printf("Success\n");
	else
		printf("Failure\n");


	printf("SizeOf(long) %li\n", sizeof(long));

}


/*
	addc(checker, 0xff);
	adds(checker, 0xddee);
	addi(checker, 0x99aabbcc);
	addl(checker, 0x1122334455667788l);
	chkc(checker, 0xff);
	chks(checker, 0xddee);
	chki(checker, 0x99aabbcc);
	chkl(checker, 0x1122334455667788l);
*/