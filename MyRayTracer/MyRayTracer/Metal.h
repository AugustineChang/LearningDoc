#pragma once
#include "Material.h"
#include "Vector3.h"

class Metal : public Material
{
public:
	Metal( const Vector3 &col );
	Metal( const Vector3 &col , float fuzzy );

	virtual bool scatter( const Ray &ray_in , const HitResult& hitResult , Vector3 &attenuation , Ray &scatteredRay ) override;

private:
	
	Vector3 albedo;
	float fuzziness;
};

