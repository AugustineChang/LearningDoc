#include "Sphere.h"
#include "MyMath.h"
#include "BoundingBox.h"
#include "Ray.h"

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
			getSphereUV( hitResult.hitPoint , realCenter , hitResult.hitUVCoord );
			return true;
		}

		float t2 = ( -b + sqrtDis ) / ( 2.0f*a );
		if ( t2 > t_min && t2 < t_max )
		{
			hitResult.t = t2;
			hitResult.hitPoint = ray.pointOnRay( t2 );
			hitResult.hitNormal = ( hitResult.hitPoint - realCenter ) / radius;
			hitResult.mat = objMat;
			getSphereUV( hitResult.hitPoint , realCenter , hitResult.hitUVCoord );
			return true;
		}
	}
	return false;
}

bool Sphere::getBoundingBox( float exposureTime , BoundingBox &aabb )
{
	if ( moveable )
	{
		Vector3 center0 = getCenterByTime( 0.0f );
		Vector3 center1 = getCenterByTime( exposureTime );

		BoundingBox box0( center0 - Vector3::oneVector * radius ,
			center0 + Vector3::oneVector * radius );
		BoundingBox box1( center1 - Vector3::oneVector * radius ,
			center1 + Vector3::oneVector * radius );

		aabb = BoundingBox( box0 , box1 );
	}
	else
	{
		aabb = BoundingBox(center - Vector3::oneVector * radius ,
			center + Vector3::oneVector * radius );
	}
	return true;
}

Vector3 Sphere::getCenterByTime( float time )
{
	return center + Vector3::upVector * MyMath::sin( time * 2.0f * PI ) * 0.06f;
}

void Sphere::getSphereUV( const Vector3 &hitPoint , const Vector3 &sphereCenter , float( &uv )[2] )
{
	Vector3 fromCenter = hitPoint - sphereCenter;
	fromCenter.normalized();

	float theta = MyMath::asin( fromCenter.y() );//[-Pi/2,Pi/2]
	float phi = MyMath::atan2( fromCenter.z() , fromCenter.x() );//[-Pi,Pi]

	uv[0] = ( phi + PI ) / ( 2.0f * PI );
	uv[1] = ( theta + PI * 0.5f ) / PI;
}

