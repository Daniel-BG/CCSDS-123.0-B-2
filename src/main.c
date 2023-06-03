#include "ccsds_1230b2_codec.h"
#include <stdlib.h>
#include <stdio.h>
#include "checker.h"
#include "debug.h"
#include "hyperspectral_image.h"
#include "metrics.h"

int main() {
	if (sizeof(long) != 8) {
		printf("Long must be 8 bytes for this to work\n");
		return -1;
	}

	//initialize image
	int * block = (int *) TEST_PATTERN;
	int bands = TEST_PATTERN_BANDS, lines = TEST_PATTERN_LINES, samples = TEST_PATTERN_SAMPLES;
	//initialize a random cube of size 16
	//int bands = 16, lines = 16, samples = 16;
	//int * block = calloc(bands*lines*samples, sizeof(int));
	//random_fill(block, bands*lines*samples, time(NULL));
	//random_fill(block, bands*lines*samples, 0); //fixed seed


	//initialize compression parameters
	CompressionParameters * cp = calloc(1, sizeof(CompressionParameters));
	set_defaults(cp);
	set_dimensions(cp, bands, lines, samples);
	reset_tables(cp);
	//uncomment the following line for lossy compression
	set_errors(14, 14, 8, 512, true, false, cp);

	
	//compress
	BitOutputStream * bos = create_bos();
	#ifdef CCSDS_CHECK_VALUES
		Checker * checker_predictor = create_checker();
		Checker * checker_encoder = create_checker();
	#else
		Checker * checker = NULL;
		Checker * checker_encoder = NULL;
	#endif
	compress(block, cp, bos, checker_predictor, checker_encoder);
	int compressed_length = bos->data->size;
	printf("From size %i downto size %i\n", cp->depth*cp->samples_per_image, compressed_length);


	//checksum
	//long expected_checksum = 0xb8c0e2c8c71bb30; //for lossless compression of the test pattern
	long expected_checksum = 0x31605ac5205db2b8; //for lossy compression set_errors(14, 14, 8, 512, true, false, cp);
	long checksum = ( (long)  get_at(bos->data, bos->data->rear)   & 0xffl 	     )|
					 (((long) get_at(bos->data, bos->data->rear-1) & 0xffl) << 8 )|
					 (((long) get_at(bos->data, bos->data->rear-2) & 0xffl) << 16)|
					 (((long) get_at(bos->data, bos->data->rear-3) & 0xffl) << 24)|
					 (((long) get_at(bos->data, bos->data->rear-4) & 0xffl) << 32)|
					 (((long) get_at(bos->data, bos->data->rear-5) & 0xffl) << 40)|
					 (((long) get_at(bos->data, bos->data->rear-6) & 0xffl) << 48)|
					 (((long) get_at(bos->data, bos->data->rear-7) & 0xffl) << 54);
	if (expected_checksum == checksum)
		printf("Expected checksum passed: %lx, (%lx)!!\n", checksum, expected_checksum);
	else
		printf("Expected checksum failed: %lx, (%lx)!!\n", checksum, expected_checksum);


	//decompress
	BitInputStream * bis = create_bis(bos->data);
	free(bos);
	recalc_encoder_params(cp);
	reset_tables(cp);
	int * decoded_block = decompress(bis, cp, checker_predictor, checker_encoder);
	printf("Decompressed with mean squared error of %lf\n", mse(block, decoded_block, cp->samples_per_image));	
}