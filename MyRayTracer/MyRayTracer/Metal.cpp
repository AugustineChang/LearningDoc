#include "Metal.h"

Metal::Metal( const Vector3 &col ) : albedo( col ) , fuzziness( 0.0f )
{
}

Metal::Metal( const Vector3 &col , float fuzzy ) : albedo( col ) , fuzziness( fuzzy )
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

	scatteredRay = Ray( hitResult.hitPoint , nextDir );
	attenuation = albedo;
	return true;
}
