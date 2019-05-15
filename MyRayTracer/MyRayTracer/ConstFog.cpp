#include "ConstFog.h"
#include "Ray.h"
#include "Hitable.h"
#include "Texture.h"

ConstFog::ConstFog( const Texture *tex ) : albedo( tex )
{
}

bool ConstFog::scatter( const Ray &ray_in , const HitResult& hitResult , Vector3 &attenuation , Ray &scatteredRay )
{
	scatteredRay = Ray( hitResult.hitPoint , Vector3::getRandomInUnitSphere() , ray_in.getSendTime() );
	attenuation = albedo->sample( hitResult.hitUVCoord[0] , hitResult.hitUVCoord[1] , hitResult.hitPoint );
	return true;
}
