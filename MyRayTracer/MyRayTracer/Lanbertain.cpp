#include "Lanbertain.h"

Lambertain::Lambertain( const Vector3 &col ) : albedo( col )
{
}

bool Lambertain::scatter( const Ray &ray_in , const HitResult& hitResult , Vector3 &attenuation , Ray &scatteredRay )
{
	Vector3 nextDir = hitResult.hitNormal + Vector3::getRandomInUnitSphere();
	nextDir.normalized();

	scatteredRay = Ray( hitResult.hitPoint , nextDir );
	attenuation = albedo;
	return true;
}
