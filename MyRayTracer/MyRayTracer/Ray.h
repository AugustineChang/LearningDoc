#pragma once
#include "Vector3.h"

class Ray
{
public:
	Ray() : origin() , direction()
	{}

	Ray( const Vector3 &a , const Vector3 &b ) 
		:origin( a ) , direction( b )
	{
	}

	Vector3 getOrigin() const { return origin; }
	Vector3 getDirection() const { return direction; }

	Vector3 pointOnRay( float t ) const
	{
		return origin + direction * t;
	}

private:

	Vector3 origin;
	Vector3 direction;
};

