#pragma once
#include "Material.h"

class Texture;

class ConstFog : public Material
{
public:
	ConstFog( const Texture *tex );

	virtual bool scatter( const Ray &ray_in , const HitResult& hitResult , Vector3 &attenuation , Ray &scatteredRay ) override;

private:

	const Texture *albedo;
};

