#include "BoundingBox.h"
#include "MyMath.h"

BoundingBox::BoundingBox() : boxMin(), boxMax()
{
}

BoundingBox::BoundingBox( const Vector3 &v_min , const Vector3 &v_max )
	: boxMin( v_min ) , boxMax( v_max )
{
}

BoundingBox::BoundingBox( const BoundingBox &boxA , const BoundingBox &boxB )
{
	float minX = min( boxA.boxMin.x() , boxB.boxMin.x() );
	float minY = min( boxA.boxMin.y() , boxB.boxMin.y() );
	float minZ = min( boxA.boxMin.z() , boxB.boxMin.z() );
	float maxX = max( boxA.boxMax.x() , boxB.boxMax.x() );
	float maxY = max( boxA.boxMax.y() , boxB.boxMax.y() );
	float maxZ = max( boxA.boxMax.z() , boxB.boxMax.z() );

	boxMin = Vector3( minX , minY , minZ );
	boxMax = Vector3( maxX , maxY , maxZ );
}

bool BoundingBox::getBoundingBox( float exposureTime , BoundingBox &aabb )
{
	aabb = *this;
	return true;
}

bool BoundingBox::isEmpty() const
{
	Vector3 diff = boxMax - boxMin;
	return ( diff.x() < 0.001f ) && ( diff.y() < 0.001f ) && ( diff.z() < 0.001f );
}

Vector3 BoundingBox::GetMin() const
{
	return boxMin;
}

Vector3 BoundingBox::GetCenter() const
{
	return ( boxMin + boxMax )*0.5f;
}

Vector3 BoundingBox::GetMax() const
{
	return boxMax;
}

bool BoundingBox::hitTest( const Ray &ray , float t_min , float t_max , HitResult& hitResult )
{
	Vector3 origin = ray.getOrigin();
	Vector3 direction = ray.getDirection();

	for ( int i = 0; i < 3; ++i )
	{
		float invertD = 1.0f / direction[i];
		float t0 = ( boxMin[i] - origin[i] ) * invertD;
		float t1 = ( boxMax[i] - origin[i] ) * invertD;
		if ( invertD < 0.0f )
			swap( t0 , t1 );
		t_min = max( t_min , t0 );
		t_max = min( t_max , t1 );
		if ( t_max <= t_min )
			return false;
	}
	
	return true;
}

void BoundingBox::swap( float &a , float &b )
{
	float temp = a;
	a = b;
	b = temp;
}
