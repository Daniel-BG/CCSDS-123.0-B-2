#include <stdio.h>
//define to enable checker (Slower operation)
//disable to disable checker (No integrity checks, faster operation)
#define CCSDS_CHECK_VALUES
//#define CCSDS_DONT_USE_HYBRID


#define DBG_EXIT {printf("ERROR @%s:%i\n", __FILE__ , __LINE__); exit(-1);}