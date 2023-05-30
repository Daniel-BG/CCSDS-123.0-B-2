#include "checker.h"


Checker * create_checker() {
	Checker * checker = calloc(1, sizeof(Checker));
	checker->queue = create_queue();
	checker->check_count = 0;
    return checker;
}

char addc(Checker * checker, char c) {
	enqueue(checker->queue, c);
	return c;
}
char chkc(Checker * checker, char c) {
	char deq = (char) dequeue(checker->queue);
	if (c != deq) {
		printf("Expected 0x%02x, got 0x%02x @%li\n", (unsigned char) deq, (unsigned char) c, checker->check_count);
	}
	checker->check_count++;
	return c;
}

short adds(Checker * checker, short s) {
	enqueue(checker->queue, (char) ((s >> 8) & 0xff));
	enqueue(checker->queue, (char) (s & 0xff));
	return s;
}
short chks(Checker * checker, short s) {
	short deq = ((((short) dequeue(checker->queue)) & 0xff) << 8)
			  | ((((short) dequeue(checker->queue)) & 0xff)); 
	if (s != deq) {
		printf("Expected 0x%04x, got 0x%04x @%li\n", (unsigned short) deq, (unsigned short) s, checker->check_count);
	}
	checker->check_count++;
	return s;
}

int addi(Checker * checker, int i) {
	enqueue(checker->queue, (char) ((i >> 24) & 0xff));
	enqueue(checker->queue, (char) ((i >> 16) & 0xff));
	enqueue(checker->queue, (char) ((i >>  8) & 0xff));
	enqueue(checker->queue, (char)  (i        & 0xff));
	return i;
}
int chki(Checker * checker, int i) {
	int deq = ((((int) dequeue(checker->queue)) & 0xff) << 24)
			  |	((((int) dequeue(checker->queue)) & 0xff) << 16)
			  | ((((int) dequeue(checker->queue)) & 0xff) << 8)
			  | ((((int) dequeue(checker->queue)) & 0xff)); 
	if (i != deq) {
		printf("Expected 0x%08x, got 0x%08x @%li\n", deq, i, checker->check_count);
	}
	checker->check_count++;
	return i;
}


long addl(Checker * checker, long l) {
	enqueue(checker->queue, (char) ((l >> 56) & 0xffl));
	enqueue(checker->queue, (char) ((l >> 48) & 0xffl));
	enqueue(checker->queue, (char) ((l >> 40) & 0xffl));
	enqueue(checker->queue, (char) ((l >> 32) & 0xffl));
	enqueue(checker->queue, (char) ((l >> 24) & 0xffl));
	enqueue(checker->queue, (char) ((l >> 16) & 0xffl));
	enqueue(checker->queue, (char) ((l >> 8) & 0xffl));
	enqueue(checker->queue, (char) ((l >> 0) & 0xffl));
	return l;
}
long chkl(Checker * checker, long l) {
	long deq =  ((((long) dequeue(checker->queue)) & 0xffl) << 56)
			  |	((((long) dequeue(checker->queue)) & 0xffl) << 48)
			  |	((((long) dequeue(checker->queue)) & 0xffl) << 40)
			  |	((((long) dequeue(checker->queue)) & 0xffl) << 32)
			  |	((((long) dequeue(checker->queue)) & 0xffl) << 24)
			  |	((((long) dequeue(checker->queue)) & 0xffl) << 16)
			  | ((((long) dequeue(checker->queue)) & 0xffl) << 8)
			  | ((((long) dequeue(checker->queue)) & 0xffl)); 
	if (l != deq) {
		printf("Expected 0x%lx, got 0x%lx @%li\n", deq, l, checker->check_count);
	}
	checker->check_count++;
	return l;
}