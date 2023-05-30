#ifndef __CHECKER_H__
#define __CHECKER_H__

#include <stdio.h>
#include "queue.h"

typedef struct {
    Queue * queue;
    long check_count;
} Checker;

Checker * create_checker();
char    addc(Checker * checker, char c);
short   adds(Checker * checker, short s);
int     addi(Checker * checker, int i);
long    addl(Checker * checker, long l);
char    chkc(Checker * checker, char c);
short   chks(Checker * checker, short s);
int     chki(Checker * checker, int i);
long    chkl(Checker * checker, long l);


#endif // __CHECKER_H__