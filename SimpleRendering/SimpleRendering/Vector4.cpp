#include "Vector4.h"

Vector4 operator*( float scale , const Vector4& v )
{
	return Vector4(
		v.vec[0] * scale ,
		v.vec[1] * scale ,
		v.vec[2] * scale ,
		v.vec[3] * scale );
}				
				
Vector4 operator/( float scale , const Vector4& v )
{
	return Vector4(
		v.vec[0] / scale ,
		v.vec[1] / scale ,
		v.vec[2] / scale ,
		v.vec[3] / scale );
}

std::ostream& operator<<( std::ostream& out , const Vector4& v )
{
	out << '(' << v.vec[0] << ',' << v.vec[1] << ',' << v.vec[2] << ',' << v.vec[3] << ')';
	return out;
}

Vector4 Vector4::operator+( const Vector4 &v2 ) const
{
	return Vector4(
		vec[0] + v2.vec[0] ,
		vec[1] + v2.vec[1] ,
		vec[2] + v2.vec[2] ,
		vec[3] + v2.vec[3] );
}

Vector4 Vector4::operator-( const Vector4 &v2 ) const
{
	return Vector4(
		vec[0] - v2.vec[0] ,
		vec[1] - v2.vec[1] ,
		vec[2] - v2.vec[2] ,
		vec[3] - v2.vec[3] );
}

Vector4 Vector4::operator*( const Vector4 &v2 ) const
{
	return Vector4(
		vec[0] * v2.vec[0] ,
		vec[1] * v2.vec[1] ,
		vec[2] * v2.vec[2] ,
		vec[3] * v2.vec[3] );
}

Vector4 Vector4::operator*( float scale ) const
{
	return Vector4(
		vec[0] * scale ,
		vec[1] * scale ,
		vec[2] * scale ,
		vec[3] * scale );
}

Vector4 Vector4::operator/( const Vector4 &v2 ) const
{
	return Vector4(
		vec[0] / v2.vec[0] ,
		vec[1] / v2.vec[1] ,
		vec[2] / v2.vec[2] ,
		vec[3] / v2.vec[3] );
}

Vector4 Vector4::operator/( float scale ) const
{
	return Vector4(
		vec[0] / scale ,
		vec[1] / scale ,
		vec[2] / scale ,
		vec[3] / scale );
}
