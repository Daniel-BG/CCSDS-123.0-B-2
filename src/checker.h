#ifndef __CHECKER_H__
#define __CHECKER_H__

#include <stdio.h>
#include "queue.h"
#include <stdbool.h>

typedef struct {
    Queue * queue;
    long check_count;
    char * message; 
    int b, l, s;
    bool failed;
} Checker;

Checker * create_checker();
Checker * set_position(Checker * checker, int b, int l, int s);
Checker * set_message(Checker * checker, char * message);


char    addc(Checker * checker, char c);
short   adds(Checker * checker, short s);
int     addi(Checker * checker, int i);
long    addl(Checker * checker, long l);
char    chkc(Checker * checker, char c);
short   chks(Checker * checker, short s);
int     chki(Checker * checker, int i);
long    chkl(Checker * checker, long l);

void burn_bytes(Checker * checker, int bytes);

void exit_if_failed(Checker * checker);


#endif // __CHECKER_H__