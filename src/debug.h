#include <stdio.h>

//Enable the intermediate value checker (performs integrity checks when compressing and decompressing, slower operation)
//#define CCSDS_CHECK_VALUES

//Use the old coder (non hybrid)
#define CCSDS_USE_HYBRID

//Perform statistic calculation (almost zero overhead)
#define CCSDS_USE_STATISTICS


#define DBG_EXIT {printf("ERROR @%s:%i\n", __FILE__ , __LINE__); exit(-1);}