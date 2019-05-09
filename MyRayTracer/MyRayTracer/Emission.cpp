#include "Emission.h"
#include "Texture.h"
#include "Ray.h"
#include "Hitable.h"

Emission::Emission( const Texture *tex ) : emit( tex )
{
}

bool Emission::scatter( const Ray &ray_in , const HitResult& hitResult , Vector3 &attenuation , Ray &scatteredRay )
{
	return false;
}

Vector3 Emission::emitted( const HitResult& hitResult )
{
	return emit->sample( hitResult.hitUVCoord[0] , hitResult.hitUVCoord[1] , hitResult.hitPoint );
}

