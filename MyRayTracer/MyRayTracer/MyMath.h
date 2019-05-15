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
	
	template<typename T>
	static T clamp( T value , T minVal , T maxVal )
	{
		if ( value < minVal )
			return minVal;
		else if ( value > maxVal )
			return maxVal;
		else
			return value;
	}

	static float lerp( float valA , float valB , float alpha );
	static float abs( float value );

	static float tan( float radian );
	static float sin( float radian );
	static float cos( float radian );

	// return [-Pi,Pi]
	static float atan2( float y , float x );
	//return [-Pi/2,Pi/2]
	static float asin( float sineVal );

	static float squareRoot( float value );
	static float power( float base , float index );
	static float log( float value );

	static int floorToInt( float value );
	static int ceilToInt( float value );

private:

	static double drand48();
	static unsigned long long seed;
};