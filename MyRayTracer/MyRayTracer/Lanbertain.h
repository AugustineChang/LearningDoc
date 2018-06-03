#pragma once
#include "Material.h"
#include "Vector3.h"

class Lanbertain : public Material
{
public:
	Lanbertain( const Vector3 &v );

	virtual bool scatter( const Ray &ray_in , const HitResult& hitResult , Vector3 & attenuation , Ray &scatteredRay ) override;

private:

	Vector3 albedo;
};

