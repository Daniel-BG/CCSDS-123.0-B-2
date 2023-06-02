#include "bit_input_stream.h"
#include "utilities.h"


BitInputStream* create_bis(Queue * data) {
	BitInputStream* bis = calloc(1, sizeof(BitInputStream));
	bis->available_bits = 0;
	bis->buffer = 0;
	bis->bits_input = 0;
	bis->data = data;
	return bis;
}

void reverse_bis(BitInputStream * bis) {
	reverse_queue(bis->data);
	apply_transform(bis->data, reverse_char_bits);
}

void free_bis(BitInputStream * bis) {
	free_queue(bis->data);
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

long reverse_read_bits(BitInputStream * bis, int quantity) {
	unsigned long result = 0;
	
	for (int i = 0; i < quantity; i++) {
		result >>= 1;
		result |= ((unsigned long) read_bit(bis)) << (sizeof(long)*8 - 1);
	}
	result >>= (sizeof(long)*8 - quantity);
	
	return result;
}


