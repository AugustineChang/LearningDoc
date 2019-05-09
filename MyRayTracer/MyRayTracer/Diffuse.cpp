#include "Diffuse.h"
#include "Ray.h"
#include "Hitable.h"
#include "Texture.h"

Diffuse::Diffuse( const Texture *tex ) : albedo( tex )
{
}

bool Diffuse::scatter( const Ray &ray_in , const HitResult& hitResult , Vector3 &attenuation , Ray &scatteredRay )
{
	Vector3 nextDir = hitResult.hitNormal + Vector3::getRandomInUnitSphere();
	nextDir.normalized();

	scatteredRay = Ray( hitResult.hitPoint , nextDir , ray_in.getSendTime() );
	attenuation = albedo->sample( hitResult.hitUVCoord[0] , hitResult.hitUVCoord[1] , hitResult.hitPoint );
	return true;
}
