#include <stdio.h>
//define to enable checker (Slower operation)
//disable to disable checker (No integrity checks, faster operation)
#define CHECK_VALUES


#define DBG_EXIT {printf("ERROR @%s:%i", __FILE__ , __LINE__); exit(-1);}