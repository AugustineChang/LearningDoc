#pragma once
#include "Ray.h"

class Material;

struct HitResult
{
	float t;//hitPoint = ray.origin + t * ray.direction;
	Vector3 hitPoint;
	Vector3 hitNormal;
	Material *mat;
};

class Hitable
{
public:
	virtual bool hitTest( const Ray &ray , float t_min , float t_max , HitResult& hitResult ) = 0;
};

