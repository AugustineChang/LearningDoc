#pragma once
#include "Hitable.h"
#include "Vector3.h"


class Sphere : public Hitable
{
public:
	Sphere()
		: center( 0.0f , 0.0f , 0.0f ) , radius( 1.0f ) , moveable( false )
	{}

	Sphere( const Vector3 &c , float r , bool isMove = false )
		: center( c ) , radius( r ) , moveable( isMove )
	{}

	virtual bool hitTest( const Ray &ray , float t_min , float t_max , HitResult& hitResult ) override;
	virtual bool getBoundingBox( float exposureTime , BoundingBox &aabb ) override;

private:

	Vector3 getCenterByTime( float time );
	void getSphereUV( const Vector3 &hitPoint , const Vector3 &sphereCenter , float( &uv )[2] );

private:

	Vector3 center;
	float radius;
	bool moveable;
};

