#pragma once
#include "Vector3.h"

class Ray
{
public:
	Ray() : origin() , direction() , sendTime( 0.0f )
	{}

	Ray( const Vector3 &a , const Vector3 &b , float time ) 
		:origin( a ) , direction( b ) , sendTime( time )
	{
	}

	Vector3 getOrigin() const { return origin; }
	Vector3 getDirection() const { return direction; }
	float	getSendTime() const { return sendTime; }

	Vector3 pointOnRay( float t ) const
	{
		return origin + direction * t;
	}

private:

	Vector3 origin;
	Vector3 direction;
	float sendTime;
};

