#pragma once
#include "Material.h"

class Vector3;
class Texture;

class Metal : public Material
{
public:
	Metal( const Texture *tex );
	Metal( const Texture *tex , float fuzzy );

	virtual bool scatter( const Ray &ray_in , const HitResult& hitResult , Vector3 &attenuation , Ray &scatteredRay ) override;

private:
	
	const Texture *albedo;
	float fuzziness;
};

