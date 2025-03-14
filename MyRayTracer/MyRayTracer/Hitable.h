#pragma once
#include "Vector3.h"

class Ray;
class Material;
class BoundingBox;

struct HitResult
{
	//hitPoint = ray.origin + t * ray.direction;
	float t;
	Vector3 hitPoint;
	Vector3 hitNormal;
	float hitUVCoord[2];
	Material *mat;
};

class Hitable
{
public:

	Hitable() : objMat( nullptr ) {}

	virtual bool hitTest( const Ray &ray , float t_min , float t_max , HitResult& hitResult ) = 0;
	virtual bool getBoundingBox( float exposureTime , BoundingBox &aabb ) = 0;

	virtual void setMaterial( Material *mat )
	{
		objMat = mat;
	}

protected:
	Material *objMat;
};

