#pragma once
#include "Material.h"

class Texture;

class Lambertain : public Material
{
public:
	Lambertain( const Texture *tex );

	virtual bool scatter( const Ray &ray_in , const HitResult& hitResult , Vector3 &attenuation , Ray &scatteredRay ) override;

private:

	const Texture *albedo;
};

