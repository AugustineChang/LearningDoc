#pragma once
#include "Vector3.h"

#define PI 3.1415926f
#define HALF_PI 1.5707963f

typedef unsigned char Byte;

class SimpleMath
{
public:

	static float Clamp01( float val )
	{
		return Clamp( val , 0.0f , 1.0f );
	}

	template<typename T>
	static T Clamp( T val , const T &min , const T &max )
	{
		return val > max ? max : ( val < min ? min : val );
	}

	template<typename T>
	static T Lerp( const T &A , const T &B , float alpha )
	{
		return (T) ( A * ( 1.0f - alpha ) + B * alpha );
	}

	template<typename T>
	static T BaryLerp( const T &A , const T &B , const T &C , const Vector3 &alpha )
	{
		return A * alpha[0] + B * alpha[1] + C * alpha[2];
	}

	template<typename T>
	static T Max( const T &A , const T &B )
	{
		return A > B ? A : B;
	}

	template<typename T>
	static T Min( const T &A , const T &B )
	{
		return A < B ? A : B;
	}

	static int floorToInt( float val );
};