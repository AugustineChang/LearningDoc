#include "Metal.h"
#include "Ray.h"
#include "Hitable.h"
#include "Texture.h"

Metal::Metal( const Texture *tex ) : albedo( tex ) , fuzziness( 0.0f )
{
}

Metal::Metal( const Texture *tex , float fuzzy ) : albedo( tex ) , fuzziness( fuzzy )
{
	if ( fuzziness > 1.0f ) fuzziness = 1.0f;
	else if ( fuzziness < 0.0f ) fuzziness = 0.0f;
}

bool Metal::scatter( const Ray &ray_in , const HitResult& hitResult , Vector3 &attenuation , Ray &scatteredRay )
{
	Vector3 nextDir = reflect( ray_in.getDirection() , hitResult.hitNormal );
	if ( fuzziness > 0.0f )
	{
		nextDir += Vector3::getRandomInUnitSphere() * fuzziness;
	}
	nextDir.normalized();

	scatteredRay = Ray( hitResult.hitPoint , nextDir , ray_in.getSendTime() );
	attenuation = albedo->sample( hitResult.hitUVCoord[0] , hitResult.hitUVCoord[1] , hitResult.hitPoint );
	return true;
}
