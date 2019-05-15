#include "VolumeFog.h"
#include "Ray.h"
#include "MyMath.h"
#include <cfloat>


VolumeFog::VolumeFog( const Vector3 &center , const Vector3 &rot , const Vector3 &extent , float density ) :
	boundary( center , rot , extent , true ) , fogDensity( density )
{
}

bool VolumeFog::hitTest( const Ray &ray , float t_min , float t_max , HitResult& hitResult )
{
	HitResult tempHit1;
	HitResult tempHit2;
	bool result = boundary.hitTest( ray , -t_max , t_max , tempHit1 );
	if ( result )
	{
		result = result && boundary.hitTest( ray , tempHit1.t + 0.0001f , t_max , tempHit2 );
	}

	if ( result )
	{
		if ( tempHit1.t < t_min )
			tempHit1.t = t_min;

		if ( tempHit2.t > t_max )
			tempHit2.t = t_max;

		if ( tempHit1.t >= tempHit2.t )
			return false;
		
		float rayDirLen = ray.getDirection().length();
		float distanceInBoundary = ( tempHit2.t - tempHit1.t ) * rayDirLen;
		float hitDistance = -( 1.0f / fogDensity ) * MyMath::log( MyMath::getRandom01() );
		if ( hitDistance <= distanceInBoundary )
		{
			hitResult.t = tempHit1.t + hitDistance / rayDirLen;
			hitResult.hitPoint = ray.pointOnRay( hitResult.t );
			hitResult.hitNormal = Vector3::zeroVector;//useless
			hitResult.hitUVCoord[0] = 0.0f;//useless
			hitResult.hitUVCoord[1] = 0.0f;//useless
			hitResult.mat = objMat;
			return true;
		}
	}

	return false;
}

bool VolumeFog::getBoundingBox( float exposureTime , BoundingBox &aabb )
{
	return boundary.getBoundingBox( exposureTime , aabb );
}
