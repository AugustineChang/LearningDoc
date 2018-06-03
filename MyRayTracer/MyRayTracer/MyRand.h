#ifndef DRAND48_H  
#define DRAND48_H  

#include <stdlib.h>  

#define m 0x100000000LL  
#define c 0xB16  
#define a 0x5DEECE66DLL  

static unsigned long long seed = 1;

double drand48( void );

void srand48( unsigned int i );

float getRandom01();

#endif 