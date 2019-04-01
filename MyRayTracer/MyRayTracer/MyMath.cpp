#include "MyMath.h"
#include <math.h>

#define m 0x100000000LL  
#define c 0xB16  
#define a 0x5DEECE66DLL  

unsigned long long MyMath::seed = 1;

double MyMath::drand48()
{
	seed = ( a * seed + c ) & 0xFFFFFFFFFFFFLL;
	unsigned int x = seed >> 16;
	return  ( (double) x / (double) m );
}

void MyMath::srand48( unsigned int i )
{
	seed = ( ( ( long long int )i ) << 16 ) | rand();
}

float MyMath::getRandom01()
{
	return static_cast<float>( drand48() );
}

float MyMath::clamp01( float value )
{
	return clamp( value , 0.0f , 1.0f );
}

float MyMath::clamp( float value , float minVal , float maxVal )
{
	if ( value < minVal )
		return minVal;
	else if ( value > maxVal )
		return maxVal;
	else
		return value;
}

float MyMath::tan( float radian )
{
	return tanf( radian );
}

float MyMath::sin( float radian )
{
	return sinf( radian );
}

float MyMath::cos( float radian )
{
	return cosf( radian );
}

float MyMath::squareRoot( float value )
{
	return sqrtf( value );
}

float MyMath::power( float base , float index )
{
	return powf( base , index );
}
