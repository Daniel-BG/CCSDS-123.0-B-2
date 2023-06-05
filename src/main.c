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
	long * block = (long *) TEST_PATTERN;
	long bands = TEST_PATTERN_BANDS, lines = TEST_PATTERN_LINES, samples = TEST_PATTERN_SAMPLES;
	//initialize a random cube of size 16
	//long bands = 16, lines = 16, samples = 16;
	//long * block = calloc(bands*lines*samples, sizeof(long));
	//random_fill(block, bands*lines*samples, time(NULL));
	//random_fill(block, bands*lines*samples, 0); //fixed seed


	//initialize compression parameters
	CompressionParameters * cp = calloc(1, sizeof(CompressionParameters));
	set_defaults(cp);
	set_dimensions(cp, bands, lines, samples);
	reset_tables(cp);
	

	//Uncomment the following lines to test different compression configurations
	long expected_checksum = 0x1de0f31ad11f0eac; 												//for lossless compression of the test pattern
	//set_errors(14, 14, 8, 0, true, false, cp); long expected_checksum = 0x2ef008594809cb82;	//lossy compression, abserr = 8
	//set_errors(14, 14, 16, 0, true, false, cp); long expected_checksum = 0x11b80717c00763fc;	//lossy compression, abserr = 16
	//set_errors(14, 14, 32, 0, true, false, cp); long expected_checksum = 0xee6010782014e5f;	//lossy compression, abserr = 32
	//set_errors(14, 14, 0, 8, false, true, cp); long expected_checksum = 0x186009b61e0c3e51;	//lossy compression, relerr = 8
	//set_errors(14, 14, 0, 16, false, true, cp); long expected_checksum = 0x1202ebdd047ce380;	//lossy compression, relerr = 16
	//set_errors(14, 14, 0, 32, false, true, cp); long expected_checksum = 0x1ab01537901ebde8;	//lossy compression, relerr = 32
	//set_errors(14, 14, 2, 2, false, true, cp); long expected_checksum = 0x178ea1551c5b4c80;	//lossy compression, abserr = relerr = 2
	//set_errors(14, 14, 4, 4, false, true, cp); long expected_checksum = 0x3ac4c65804934440;	//lossy compression, abserr = relerr = 2
	//set_errors(14, 14, 8, 8, false, true, cp); long expected_checksum = 0x186009b61e0c3e51;	//lossy compression, abserr = relerr = 2

	
	//compress
	BitOutputStream * bos = create_bos();
	#ifdef CCSDS_CHECK_VALUES
		Checker * checker_predictor = create_checker();
		Checker * checker_encoder = create_checker();
	#else
		Checker * checker_predictor = NULL;
		Checker * checker_encoder = NULL;
	#endif
	compress(block, cp, bos, checker_predictor, checker_encoder);
	long compressed_length = bos->data->size;
	printf("From size %li downto size %li (%f%%)\n", cp->depth*cp->samples_per_image, compressed_length, 100.0 * (double)compressed_length / (double) (cp->depth*cp->samples_per_image));
	
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
	long * decoded_block = decompress(bis, cp, checker_predictor, checker_encoder);
	printf("Decompressed with mean squared error of %lf\n", mse(block, decoded_block, cp->samples_per_image));	
}