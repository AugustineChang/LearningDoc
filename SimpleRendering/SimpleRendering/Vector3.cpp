#include "Vector3.h"
#include <math.h>

Vector3::Vector3()
{
	vec[0] = 0.0f;
	vec[1] = 0.0f;
	vec[2] = 0.0f;
}


Vector3::Vector3( float x , float y , float z )
{
	vec[0] = x;
	vec[1] = y;
	vec[2] = z;
}

Vector3 Vector3::operator+( const Vector3 &v2 ) const
{
	return Vector3( 
		vec[0] + v2.vec[0] , 
		vec[1] + v2.vec[1] ,
		vec[2] + v2.vec[2] );
}

Vector3 Vector3::operator-( const Vector3 &v2 ) const
{
	return Vector3(
		vec[0] - v2.vec[0] ,
		vec[1] - v2.vec[1] ,
		vec[2] - v2.vec[2] );
}

Vector3 Vector3::operator-() const
{
	return Vector3( -vec[0] , -vec[1] , -vec[2] );
}

Vector3 Vector3::operator*( const Vector3 &v2 ) const
{
	return Vector3(
		vec[0] * v2.vec[0] ,
		vec[1] * v2.vec[1] ,
		vec[2] * v2.vec[2] );
}

Vector3 Vector3::operator*( float scale ) const
{
	return Vector3(
		vec[0] * scale ,
		vec[1] * scale ,
		vec[2] * scale );
}

Vector3 Vector3::operator/( const Vector3 &v2 ) const
{
	return Vector3(
		vec[0] / v2.vec[0] ,
		vec[1] / v2.vec[1] ,
		vec[2] / v2.vec[2] );
}

Vector3 Vector3::operator/( float scale ) const
{
	return Vector3(
		vec[0] / scale ,
		vec[1] / scale ,
		vec[2] / scale );
}

void Vector3::normalized()
{
	float invSqrt = 1.0f / length();
	*this *= invSqrt;
}

float Vector3::length() const
{
	return sqrtf( vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] );
}

float Vector3::lengthSquared() const
{
	return vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2];
}

Vector3 Vector3::unitVector() const
{
	float invSqrt = 1.0f / length();
	return *this * invSqrt;
}

float Vector3::dot( const Vector3& v1 , const Vector3& v2 )
{
	return v1.vec[0] * v2.vec[0] + v1.vec[1] * v2.vec[1] + v1.vec[2] * v2.vec[2];
}

Vector3 Vector3::cross( const Vector3& v1 , const Vector3& v2 )
{
	return Vector3(
		v1.vec[1] * v2.vec[2] - v2.vec[1] * v1.vec[2] , // y1*z2 - y2*z1
		v1.vec[2] * v2.vec[0] - v2.vec[2] * v1.vec[0] , // z1*x2 - z2*x1
		v1.vec[0] * v2.vec[1] - v2.vec[0] * v1.vec[1]   // x1*y2 - x2*y1
	);
}

void Vector3::operator*=( const Vector3 &v2 )
{
	vec[0] *= v2.vec[0];
	vec[1] *= v2.vec[1];
	vec[2] *= v2.vec[2];
}

void Vector3::operator-=( const Vector3 &v2 )
{
	vec[0] -= v2.vec[0];
	vec[1] -= v2.vec[1];
	vec[2] -= v2.vec[2];
}

void Vector3::operator+=( const Vector3 &v2 )
{
	vec[0] += v2.vec[0];
	vec[1] += v2.vec[1];
	vec[2] += v2.vec[2];
}

void Vector3::operator/=( const Vector3 &v2 )
{
	vec[0] /= v2.vec[0];
	vec[1] /= v2.vec[1];
	vec[2] /= v2.vec[2];
}

void Vector3::operator*=( float scale )
{
	vec[0] *= scale;
	vec[1] *= scale;
	vec[2] *= scale;
}

void Vector3::operator/=( float scale )
{
	vec[0] /= scale;
	vec[1] /= scale;
	vec[2] /= scale;
}

Vector3 operator*( float scale , const Vector3& v )
{
	return Vector3(
		v.vec[0] * scale ,
		v.vec[1] * scale ,
		v.vec[2] * scale );
}

Vector3 operator/( float scale , const Vector3& v )
{
	return Vector3(
		v.vec[0] / scale ,
		v.vec[1] / scale ,
		v.vec[2] / scale );
}

std::ostream& operator<<( std::ostream& out , const Vector3& v )
{
	out << '(' << v.vec[0] << ',' << v.vec[1] << ',' << v.vec[2] << ')';
	return out;
}
