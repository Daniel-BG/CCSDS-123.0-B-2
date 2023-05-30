#ifndef __BIT_INPUT_STREAM_H__
#define __BIT_INPUT_STREAM_H__

#include "queue.h"

typedef struct {
  	Queue * data;
  	int available_bits;
  	int buffer;
  	long bits_input;
} BitInputStream;


BitInputStream* create_bis(Queue * data);
void free_bis(BitInputStream * bis);
char read_bit(BitInputStream * bis);
long read_bits(BitInputStream * bis, int quantity);
	

#endif //__BIT_INPUT_STREAM_H__