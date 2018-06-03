#pragma once
#include "Hitable.h"
#include "Vector3.h"

class Sphere : public Hitable
{
public:
	Sphere()
		: center( 0.0f , 0.0f , 0.0f ) , radius( 1.0f )
	{}

	Sphere( const Vector3 &c , float r )
		: center( c ) , radius( r )
	{}

	virtual bool hitTest( const Ray &ray , float t_min , float t_max , HitResult& hitResult ) override;

private:

	Vector3 center;
	float radius;
};

