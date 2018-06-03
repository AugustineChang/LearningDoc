#include "Lanbertain.h"

Lanbertain::Lanbertain( const Vector3 &v )
	: albedo( v )
{
}

bool Lanbertain::scatter( const Ray &ray_in , const HitResult& hitResult , Vector3 &attenuation , Ray &scatteredRay )
{
	Vector3 nextDir = hitResult.hitNormal + Vector3::getRandomInUnitSphere();
	scatteredRay = Ray( hitResult.hitPoint , nextDir );
	attenuation = albedo;
	return true;
}
