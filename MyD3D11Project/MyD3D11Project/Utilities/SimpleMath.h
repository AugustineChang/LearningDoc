#pragma once
class SimpleMath
{
public:
	
	template<typename T>
	static T Clamp( T value , T min , T max )
	{
		return value < min ? min : ( value > max ? max : value );
	}

	// Returns random float in [0, 1).
	static float RandF()
	{
		return (float) ( rand() ) / (float) RAND_MAX;
	}

	// Returns random float in [a, b).
	static float RandF( float a , float b )
	{
		return a + RandF()*( b - a );
	}

public:

	static const float PI;
};

template<typename T>
struct Point
{
	T x;
	T y;
};