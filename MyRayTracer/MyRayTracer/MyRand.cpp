#include "MyRand.h"

double drand48( void )
{
	seed = ( a * seed + c ) & 0xFFFFFFFFFFFFLL;
	unsigned int x = seed >> 16;
	return  ( (double) x / (double) m );

}

void srand48( unsigned int i )
{
	seed = ( ( ( long long int )i ) << 16 ) | rand();
}

float getRandom01()
{
	return static_cast<float>( drand48() );
}