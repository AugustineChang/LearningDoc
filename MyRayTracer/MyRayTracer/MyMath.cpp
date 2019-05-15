#include "MyMath.h"
#include <math.h>

#define m 0x100000000LL  
#define c 0xB16  
#define a 0x5DEECE66DLL  

unsigned long long MyMath::seed = 1;

double MyMath::drand48()
{
	seed = ( a * seed + c ) & 0xFFFFFFFFFFFFLL;
	unsigned int x = static_cast<unsigned int>( seed >> 16 );
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

int MyMath::getRandomInteger( int maxVal )
{
	return floorToInt( getRandom01() * maxVal );
}

float MyMath::clamp01( float value )
{
	return clamp<float>( value , 0.0f , 1.0f );
}

float MyMath::lerp( float valA , float valB , float alpha )
{
	return valA + ( valB - valA ) * alpha;
}

float MyMath::abs( float value )
{
	return fabsf( value );
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

float MyMath::atan2( float y , float x )
{
	return atan2f( y , x );
}

float MyMath::asin( float sineVal )
{
	return asinf( sineVal );
}

float MyMath::squareRoot( float value )
{
	return sqrtf( value );
}

float MyMath::power( float base , float index )
{
	return powf( base , index );
}

float MyMath::log( float value )
{
	return logf( value );
}

int MyMath::floorToInt( float value )
{
	return static_cast<int>( floor( value ) );
}

int MyMath::ceilToInt( float value )
{
	return static_cast<int>( ceil( value ) );
}