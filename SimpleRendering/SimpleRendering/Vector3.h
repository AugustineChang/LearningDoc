#pragma once
#include <iostream>

class Vector3
{
public:
	Vector3();
	Vector3( float x , float y , float z );//左手系 x向前 y向右 z向上

	float x() const { return vec[0]; }
	float y() const { return vec[1]; }
	float z() const { return vec[2]; }

	Vector3 operator-() const;

	Vector3 operator+( const Vector3 &v2 ) const;
	Vector3 operator-( const Vector3 &v2 ) const;
	Vector3 operator*( const Vector3 &v2 ) const;
	Vector3 operator/( const Vector3 &v2 ) const;

	Vector3 operator*( float scale ) const;
	Vector3 operator/( float scale ) const;
	friend Vector3 operator*( float scale , const Vector3& v );
	friend Vector3 operator/( float scale , const Vector3& v );

	void operator+=( const Vector3 &v2 );
	void operator-=( const Vector3 &v2 );
	void operator*=( const Vector3 &v2 );
	void operator/=( const Vector3 &v2 );
	void operator*=( float scale );
	void operator/=( float scale );

	float operator[]( int index ) const
	{
		return vec[index];
	}

	float& operator[]( int index )
	{
		return vec[index];
	}

	friend std::ostream& operator<<( std::ostream& out , const Vector3& v );

public:

	void normalized();
	float length() const;
	float lengthSquared() const;
	Vector3 unitVector() const;

	static float dot( const Vector3& v1 , const Vector3& v2 );
	static Vector3 cross( const Vector3& v1 , const Vector3& v2 );

private:

	float vec[3];

};

Vector3 operator*( float scale , const Vector3& v );
Vector3 operator/( float scale , const Vector3& v );
std::ostream& operator<<( std::ostream& out , const Vector3& v );