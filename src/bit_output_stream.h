#ifndef __BIT_OUTPUT_STREAM_H__
#define __BIT_OUTPUT_STREAM_H__

#define _BOS_LONG_LEFT_BIT_MASK (0x1l << (sizeof(long)*8-1))


#include "queue.h"

typedef struct {
  	Queue * data;
	int buffer;
	int buffer_size;
	long bits_output; //enough size not to ever overflow
} BitOutputStream;


BitOutputStream* create_bos();
void free_bos(BitOutputStream * bos);
void write_bit(BitOutputStream * bos, long bit);
void write_bits(BitOutputStream * bos, long bits, int quantity);

#endif