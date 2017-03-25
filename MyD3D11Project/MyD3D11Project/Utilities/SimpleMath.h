#pragma once
#include <DirectXMath.h>
#include <stdlib.h>

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

	inline static DirectX::XMFLOAT4 Add( const DirectX::XMFLOAT4 &a , const DirectX::XMFLOAT4 &b )
	{
		return DirectX::XMFLOAT4( a.x + b.x , a.y + b.y , a.z + b.z , a.w + b.w );
	}

	inline static DirectX::XMFLOAT4 Sub( const DirectX::XMFLOAT4 &a , const DirectX::XMFLOAT4 &b )
	{
		return DirectX::XMFLOAT4( a.x - b.x , a.y - b.y , a.z - b.z , a.w - b.w );
	}

	inline static DirectX::XMFLOAT4 Mul( const DirectX::XMFLOAT4 &a , const DirectX::XMFLOAT4 &b )
	{
		return DirectX::XMFLOAT4( a.x * b.x , a.y * b.y , a.z * b.z , a.w * b.w );
	}

	inline static DirectX::XMFLOAT4 Mul( const DirectX::XMFLOAT4 &a , float scale )
	{
		return DirectX::XMFLOAT4( a.x * scale , a.y * scale , a.z * scale , a.w * scale );
	}

	inline static DirectX::XMFLOAT4 Div( const DirectX::XMFLOAT4 &a , const DirectX::XMFLOAT4 &b )
	{
		return DirectX::XMFLOAT4( a.x / b.x , a.y / b.y , a.z / b.z , a.w / b.w );
	}

	inline static DirectX::XMFLOAT2 Div( const DirectX::XMFLOAT2 &a , const DirectX::XMFLOAT2 &b )
	{
		return DirectX::XMFLOAT2( a.x / b.x , a.y / b.y );
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