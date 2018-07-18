#pragma once
#include "Vector3.h"


class Vector4
{
public:

	Vector4()
	{
		vec[0] = 0.0f;
		vec[1] = 0.0f;
		vec[2] = 0.0f;
		vec[3] = 1.0f;
	}

	Vector4( int x , int y , int z , int w )
	{
		vec[0] = float( x );
		vec[1] = float( y );
		vec[2] = float( z );
		vec[3] = float( w );
	}

	Vector4( float x , float y , float z , float w )
	{
		vec[0] = x;
		vec[1] = y;
		vec[2] = z;
		vec[3] = w;
	}

	Vector4( const Vector3 &v3 , float w )
	{
		vec[0] = v3.x();
		vec[1] = v3.y();
		vec[2] = v3.z();
		vec[3] = w;
	}

	float operator[]( int index ) const
	{ 
		return vec[index];
	}

	float& operator[]( int index )
	{
		return vec[index];
	}

	Vector3 getVector3() const 
	{
		return Vector3( vec[0] , vec[1] , vec[2] );
	}

	friend std::ostream& operator<<( std::ostream& out , const Vector4& v );

	Vector4 operator+( const Vector4 &v2 ) const;
	Vector4 operator-( const Vector4 &v2 ) const;
	Vector4 operator*( const Vector4 &v2 ) const;
	Vector4 operator/( const Vector4 &v2 ) const;

	Vector4 operator*( float scale ) const;
	Vector4 operator/( float scale ) const;
	friend Vector4 operator*( float scale , const Vector4& v );
	friend Vector4 operator/( float scale , const Vector4& v );

private:

	float vec[4];
};

Vector4 operator*( float scale , const Vector4& v );
Vector4 operator/( float scale , const Vector4& v );
std::ostream& operator<<( std::ostream& out , const Vector4& v );