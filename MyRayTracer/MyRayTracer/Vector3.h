#pragma once
#include <iostream>

class Vector3
{
	friend Vector3 operator*( float scale , const Vector3& v );
	friend Vector3 operator/( float scale , const Vector3& v );

	friend std::istream& operator>>( std::istream& in , Vector3 &vec );
	friend std::ostream& operator<<( std::ostream& out , const Vector3 &vec );

public:
	Vector3() 
	{
		vec[0] = 0.0f;
		vec[1] = 0.0f;
		vec[2] = 0.0f;
	}

	Vector3( float e0 , float e1 , float e2 )
	{
		vec[0] = e0;
		vec[1] = e1;
		vec[2] = e2;
	}

	Vector3( const Vector3& copy )
	{
		vec[0] = copy.vec[0];
		vec[1] = copy.vec[1];
		vec[2] = copy.vec[2];
	}

	float x() const { return vec[0]; }
	float y() const { return vec[1]; }
	float z() const { return vec[2]; }

	const Vector3& operator+() const { return *this; }
	Vector3 operator-() const { return Vector3( -vec[0], -vec[1] , -vec[2] ); }
	float operator[]( int index ) const { return vec[index]; }
	float& operator[]( int index ) { return vec[index]; }

	Vector3 operator+( const Vector3& vec2 ) const;
	Vector3 operator-( const Vector3& vec2 ) const;
	Vector3 operator*( const Vector3& vec2 ) const;
	Vector3 operator/( const Vector3& vec2 ) const;
	Vector3 operator*( float scale ) const;
	Vector3 operator/( float scale ) const;

	Vector3& operator+=( const Vector3& vec2 );
	Vector3& operator-=( const Vector3& vec2 );
	Vector3& operator*=( const Vector3& vec2 );
	Vector3& operator/=( const Vector3& vec2 );
	Vector3& operator*=( float scale );
	Vector3& operator/=( float scale );


	float length() const;
	float squared_length() const;
	void normalized();

	static float dot( const Vector3& v1 , const Vector3& v2 );
	static Vector3 cross( const Vector3& v1 , const Vector3& v2 );
	static Vector3 lerp( const Vector3& v1 , const Vector3& v2 , float alpha );
	static Vector3 getRandomInUnitSphere();

private:

	float vec[3];
};

Vector3 operator*( float scale , const Vector3 &v );
Vector3 operator/( float scale , const Vector3 &v );
std::istream& operator>>( std::istream& in , Vector3 &vec );
std::ostream& operator<<( std::ostream& out , const Vector3 &vec );