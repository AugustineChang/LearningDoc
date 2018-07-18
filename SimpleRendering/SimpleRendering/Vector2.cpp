#include "Vector2.h"
#include "SimpleMath.h"

Vector2 Vector2::operator+( const Vector2 &v2 ) const
{
	return Vector2( vec[0] + v2.vec[0] , vec[1] + v2.vec[1] );
}

Vector2 Vector2::operator-( const Vector2 &v2 ) const
{
	return Vector2( vec[0] - v2.vec[0] , vec[1] - v2.vec[1] );
}

Vector2 Vector2::operator*( const Vector2 &v2 ) const
{
	return Vector2( vec[0] * v2.vec[0] , vec[1] * v2.vec[1] );
}

Vector2 Vector2::operator*( float scale ) const
{
	return Vector2( vec[0] * scale , vec[1] * scale );
}

Vector2 Vector2::operator/( const Vector2 &v2 ) const
{
	return Vector2( vec[0] / v2.vec[0] , vec[1] / v2.vec[1] );
}

Vector2 Vector2::operator/( float scale ) const
{
	return Vector2( vec[0] / scale , vec[1] / scale );
}

void Vector2::clamp01()
{
	vec[0] = SimpleMath::Clamp01( vec[0] );
	vec[1] = SimpleMath::Clamp01( vec[1] );
}

Vector2 operator*( float scale , const Vector2& v )
{
	return Vector2( v.vec[0] * scale , v.vec[1] * scale );
}

Vector2 operator/( float scale , const Vector2& v )
{
	return Vector2( v.vec[0] / scale , v.vec[1] / scale );
}
