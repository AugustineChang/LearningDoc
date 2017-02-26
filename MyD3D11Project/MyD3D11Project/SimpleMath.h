#pragma once
class SimpleMath
{
public:
	
	template<typename T>
	static T Clamp( T value , T min , T max )
	{
		return value < min ? min : ( value > max ? max : value );
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