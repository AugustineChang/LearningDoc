#pragma once
#include "Material.h"
class Glass : public Material
{
public:
	Glass( float refra_index );
	Glass( const Vector3 &color , float refra_index );

	virtual bool scatter( const Ray &ray_in , const HitResult& hitResult , Vector3 &attenuation , Ray &scatteredRay ) override;

private:

	float schilick( float cosine );

	float refractiveIndex; //’€…‰¬  air = 1, glass = 1.3-1.7, diamond = 2.4
	Vector3 albedo;
};

