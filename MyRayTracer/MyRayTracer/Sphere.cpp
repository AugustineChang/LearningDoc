#include "Sphere.h"
#include "MyMath.h"

bool Sphere::hitTest( const Ray &ray , float t_min , float t_max , HitResult& hitResult )
{
	Vector3 realCenter = moveable ? getCenterByTime( ray.getSendTime() ) : center;

	float a = Vector3::dot( ray.getDirection() , ray.getDirection() );
	Vector3 center2Origin = ray.getOrigin() - realCenter;
	float b = 2.0f * Vector3::dot( ray.getDirection() , center2Origin );
	float c = Vector3::dot( center2Origin , center2Origin ) - radius * radius;

	float discriminant = b * b - 4.0f * a * c;
	if ( discriminant > 0 )
	{
		float sqrtDis = MyMath::squareRoot( discriminant );
		float t1 = ( -b - sqrtDis ) / ( 2.0f*a );
		if ( t1 > t_min && t1 < t_max )
		{
			hitResult.t = t1;
			hitResult.hitPoint = ray.pointOnRay( t1 );
			hitResult.hitNormal = ( hitResult.hitPoint - realCenter ) / radius;
			hitResult.mat = objMat;
			return true;
		}

		float t2 = ( -b + sqrtDis ) / ( 2.0f*a );
		if ( t2 > t_min && t2 < t_max )
		{
			hitResult.t = t2;
			hitResult.hitPoint = ray.pointOnRay( t2 );
			hitResult.hitNormal = ( hitResult.hitPoint - realCenter ) / radius;
			hitResult.mat = objMat;
			return true;
		}
	}
	return false;
}

Vector3 Sphere::getCenterByTime( float time )
{
	return center + Vector3::upVector * MyMath::sin( time * 2.0f * PI ) * 0.06f;
}
