#include "bit_input_stream.h"


BitInputStream* create_bis(Queue * data) {
	BitInputStream* bis = (BitInputStream*) malloc(sizeof(BitInputStream));
	bis->available_bits = 0;
	bis->buffer = 0;
	bis->bits_input = 0;
	return bis;
}

char read_bit(BitInputStream * bis) {
	if (bis->available_bits == 0) {
		bis->buffer = dequeue(bis->data);
		bis->available_bits = 8;
	}
	char result = (bis->buffer & 0x80) ? 1 : 0;
	bis->buffer <<= 1;
	bis->available_bits--;
	bis->bits_input++;
	return result;	 
}
	
long read_bits(BitInputStream * bis, int quantity) {
	long result = 0;

	for (int i = 0; i < quantity; i++) {
		result <<= 1;
		result |= read_bit(bis);
	}

	return result;
}
