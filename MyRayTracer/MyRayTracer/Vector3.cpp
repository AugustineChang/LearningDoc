#include "Vector3.h"
#include "MyRand.h"
#include <math.h>

Vector3 Vector3::operator+( const Vector3& v2 ) const
{
	return Vector3( vec[0] + v2.vec[0] , vec[1] + v2.vec[1] , vec[2] + v2.vec[2] );
}

Vector3 Vector3::operator-( const Vector3& v2 ) const
{
	return Vector3( vec[0] - v2.vec[0] , vec[1] - v2.vec[1] , vec[2] - v2.vec[2] );
}

Vector3 Vector3::operator*( const Vector3& v2 ) const
{
	return Vector3( vec[0] * v2.vec[0] , vec[1] * v2.vec[1] , vec[2] * v2.vec[2] );
}

Vector3 Vector3::operator*( float scale ) const
{
	return Vector3( vec[0] * scale , vec[1] * scale , vec[2] * scale );
}

Vector3 Vector3::operator/( const Vector3& v2 ) const
{
	return Vector3( vec[0] / v2.vec[0] , vec[1] / v2.vec[1] , vec[2] / v2.vec[2] );
}

Vector3 Vector3::operator/( float scale ) const
{
	return Vector3( vec[0] / scale , vec[1] / scale , vec[2] / scale );
}

float Vector3::length() const
{
	return sqrtf( vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] );
}

float Vector3::squared_length() const
{
	return vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2];
}

void Vector3::normalized()
{
	float invSqrt = 1.0f / length();
	vec[0] *= invSqrt;
	vec[1] *= invSqrt;
	vec[2] *= invSqrt;
}

float Vector3::dot( const Vector3& v1 , const Vector3& v2 )
{
	return v1.vec[0] * v2.vec[0] + v1.vec[1] * v2.vec[1] + v1.vec[2] * v2.vec[2];
}

Vector3 Vector3::cross( const Vector3& v1 , const Vector3& v2 )
{
	return Vector3(
		v1.vec[1] * v2.vec[2] - v1.vec[2] * v2.vec[1] ,
		v1.vec[2] * v2.vec[0] - v1.vec[0] * v2.vec[2] ,
		v1.vec[0] * v2.vec[1] - v1.vec[1] * v2.vec[0]
	);
}

Vector3 Vector3::lerp( const Vector3& v1 , const Vector3& v2 , float alpha )
{
	return ( 1.0f - alpha ) * v1 + alpha * v2;
}

Vector3 Vector3::getRandomInUnitSphere()
{
	Vector3 randVec;
	do
	{
		randVec = 2.0f * Vector3( getRandom01() , getRandom01() , getRandom01() ) - Vector3( 1.0f , 1.0f , 1.0f );
	} while ( randVec.squared_length() >= 1.0f );

	return randVec;
}

Vector3 Vector3::getRandomInDisk()
{
	Vector3 randVec;
	do
	{
		randVec = 2.0f * Vector3( getRandom01() , getRandom01() , 0.0f ) - Vector3( 1.0f , 1.0f , 0.0f );
	} while ( randVec.squared_length() >= 1.0f );

	return randVec;
}

Vector3 Vector3::getRandomColor()
{
	return Vector3( getRandom01() , getRandom01() , getRandom01() );
}

Vector3& Vector3::operator/=( const Vector3& v2 )
{
	vec[0] /= v2.vec[0];
	vec[1] /= v2.vec[1];
	vec[2] /= v2.vec[2];

	return *this;
}

Vector3& Vector3::operator*=( const Vector3& v2 )
{
	vec[0] *= v2.vec[0];
	vec[1] *= v2.vec[1];
	vec[2] *= v2.vec[2];

	return *this;
}

Vector3& Vector3::operator-=( const Vector3& v2 )
{
	vec[0] -= v2.vec[0];
	vec[1] -= v2.vec[1];
	vec[2] -= v2.vec[2];

	return *this;
}

Vector3& Vector3::operator+=( const Vector3& v2 )
{
	vec[0] += v2.vec[0];
	vec[1] += v2.vec[1];
	vec[2] += v2.vec[2];

	return *this;
}

Vector3& Vector3::operator*=( float scale )
{
	vec[0] *= scale;
	vec[1] *= scale;
	vec[2] *= scale;

	return *this;
}

Vector3& Vector3::operator/=( float scale )
{
	vec[0] /= scale;
	vec[1] /= scale;
	vec[2] /= scale;

	return *this;
}

//////////////////////////////////////////////////////////////////////////////////////

Vector3 operator*( float scale , const Vector3 &v )
{
	return Vector3( v.vec[0] * scale , v.vec[1] * scale , v.vec[2] * scale );
}

Vector3 operator/( float scale , const Vector3 &v )
{
	return Vector3( v.vec[0] / scale , v.vec[1] / scale , v.vec[2] / scale );
}

std::istream& operator>>( std::istream& in , Vector3 &v )
{
	in >> v.vec[0] >> v.vec[1] >> v.vec[2];
	return in;
}

std::ostream& operator<<( std::ostream& out , const Vector3 &v )
{
	out << v.vec[0] << ' ' << v.vec[1] << ' ' << v.vec[2];
	return out;
}