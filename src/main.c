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
	//initialize a random cube of size 128
	//long bands = 128, lines = 128, samples = 128;
	//long * block = calloc(bands*lines*samples, sizeof(long));
	//random_fill(block, bands*lines*samples, 0); //fixed seed
	//sequential_fill(block, bands*lines*samples);


	//initialize compression parameters
	CompressionParameters * cp = calloc(1, sizeof(CompressionParameters));
	set_defaults(cp);
	set_dimensions(cp, bands, lines, samples);
	reset_tables(cp);
	reset_stats(cp);

	//Uncomment the following lines to test different compression configurations (For the TEST IMAGE)
	long expected_checksum = 0x1de0f31ad11f0eac; 												//for lossless compression of the test pattern
	//set_errors(14, 14, 8, 0, true, false, cp); long expected_checksum = 0x2ef008594809cb82;	//lossy compression, abserr = 8
	//set_errors(14, 14, 16, 0, true, false, cp); long expected_checksum = 0x11b80717c00763fc;	//lossy compression, abserr = 16
	//set_errors(14, 14, 32, 0, true, false, cp); long expected_checksum = 0xee6010782014e5f;	//lossy compression, abserr = 32
	//set_errors(14, 14, 0, 8, false, true, cp); long expected_checksum = 0x186009b61e0c3e51;	//lossy compression, relerr = 8
	//set_errors(14, 14, 0, 16, false, true, cp); long expected_checksum = 0x1202ebdd047ce380;	//lossy compression, relerr = 16
	//set_errors(14, 14, 0, 32, false, true, cp); long expected_checksum = 0x1ab01537901ebde8;	//lossy compression, relerr = 32
	//set_errors(14, 14, 2, 2, true, true, cp); long expected_checksum = 0x178ea1551c5b4c80;	//lossy compression, abserr = relerr = 2
	//set_errors(14, 14, 4, 4, true, true, cp); long expected_checksum = 0x3ac4c65804934440;	//lossy compression, abserr = relerr = 4
	//set_errors(14, 14, 8, 8, true, true, cp); long expected_checksum = 0x186009b61e0c3e51;	//lossy compression, abserr = relerr = 8

	//Uncomment the following lines to test different compression configurations (For the SEQUENTIAL FILL size 128x128x128)
	//long expected_checksum = 0x40000100000180; 												//for lossless compression of the test pattern
	//set_errors(14, 14, 2, 2, true, true, cp); long expected_checksum = 0x800000800000c0;	//lossy compression, abserr = relerr = 2
	//set_errors(14, 14, 4, 4, true, true, cp); long expected_checksum = 0x2000002000003;	//lossy compression, abserr = relerr = 4
	//set_errors(14, 14, 8, 8, true, true, cp); long expected_checksum = 0x40000110000078;	//lossy compression, abserr = relerr = 8

	//Uncomment the following lines to test different compression configurations (For the RANDOM FILL size 128x128x128 seed 0)
	//long expected_checksum = 0x2bc07f1e41b19f80; 											//for lossless compression of the test pattern
	//set_errors(14, 14, 2, 2, true, true, cp); long expected_checksum = 0x12594a39096ecb7c;	//lossy compression, abserr = relerr = 2
	//set_errors(14, 14, 4, 4, true, true, cp); long expected_checksum = 0xfd17b6db1750d28;	//lossy compression, abserr = relerr = 4
	//set_errors(14, 14, 8, 8, true, true, cp); long expected_checksum = 0x1dc530f805fb9140;	//lossy compression, abserr = relerr = 8

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
	long compressed_length = bos->data->size * 8;
	printf("From size %li downto size %li (%f%%)\n", cp->depth*cp->samples_per_image, compressed_length, 100.0 * (double)compressed_length / (double) (cp->depth*cp->samples_per_image));
	printf("Compressed bits are %li raw mqi, %li golomb rem, %li golomb unary, %li acc bits, %li tablecw bits, %li table flush, %li accumulator flush, %li end bit\n", cp->stats_mqi, cp->stats_golombrem, cp->stats_golombunary, cp->stats_accbit, cp->stats_tablecwbits, cp->stats_tableflush, cp->stats_accflush, cp->stats_endbit);


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