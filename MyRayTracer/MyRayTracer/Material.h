#pragma once
#include "Hitable.h"

class Material
{
public:
	virtual bool scatter( const Ray &ray_in , const HitResult& hitResult , Vector3 &attenuation , Ray &scatteredRay ) = 0;
};