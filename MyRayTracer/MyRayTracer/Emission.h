#pragma once
#include "Material.h"

class Texture;

class Emission : public Material
{
public:
	Emission( const Texture *tex );
	
	virtual bool scatter( const Ray &ray_in , const HitResult& hitResult , Vector3 &attenuation , Ray &scatteredRay ) override;
	virtual Vector3 emitted( const HitResult& hitResult ) override;

private:
	const Texture *emit;
};

