#pragma once

#include <stdlib.h>

#define PI       3.1415926f

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

class MyMath
{
public:
	
	static void srand48( unsigned int i );
	static float getRandom01();
	static int getRandomInteger( int maxVal );

	static float clamp01( float value );
	static float clamp( float value , float minVal , float maxVal );
	static float lerp( float valA , float valB , float alpha );
	static float abs( float value );

	static float tan( float radian );
	static float sin( float radian );
	static float cos( float radian );

	static float squareRoot( float value );
	static float power( float base , float index );

	static int floorToInt( float value );
	static int ceilToInt( float value );

private:

	static double drand48();
	static unsigned long long seed;
};