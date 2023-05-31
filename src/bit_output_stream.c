#include "bit_output_stream.h"


BitOutputStream* create_bos() {
	BitOutputStream* bos = calloc(1, sizeof(BitOutputStream));
	bos->data = create_queue();
	bos->buffer = 0;
	bos->buffer_size = 0;
	bos->bits_output = 0;
    return bos;
}

void free_bos(BitOutputStream * bos) {
	free_queue(bos->data);
}

void write_bit(BitOutputStream * bos, long bit) {	
	//normalize to 0/1
	if (bit) bit = 1;

	bos->buffer <<= 1;
	bos->buffer |= (char) bit;
	bos->buffer_size ++;
	if (bos->buffer_size == 8) {
		enqueue(bos->data, bos->buffer);
		bos->buffer = 0;
		bos->buffer_size = 0;
	}
	bos->bits_output++;
}


void write_bits(BitOutputStream * bos, long bits, int quantity) { 
	//adjust the bits so that the first one is in the leftmost position
	bits <<= sizeof(long)*8 - quantity;
	for (int i = 0; i < quantity; i++) {
		write_bit(bos, bits & _BOS_LONG_LEFT_BIT_MASK);
		bits <<= 1;
	}

}

void flush(BitOutputStream * bos) {
	while(bos->buffer_size != 0)
		write_bit(bos, 0l);
}