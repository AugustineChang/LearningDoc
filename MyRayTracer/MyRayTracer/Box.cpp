#include "Box.h"
#include "Ray.h"
#include "RotateMatrix.h"
#include "BoundingBox.h"
#include "AxisAlignedRect.h"

Box::Box( const Vector3 &pos , bool twoSide ) :
	isTwoSide( twoSide ) , center( pos ) , extent( Vector3::oneVector * 0.5f ) , matrix() , invertMatrix()
{
	createSurfaces();
}

Box::Box( const Vector3 &pos , const Vector3 &rot , bool twoSide ) :
	isTwoSide( twoSide ) , center( pos ) , extent( Vector3::oneVector * 0.5f ) , matrix( rot ) ,
	invertMatrix( rot )
{
	createSurfaces();
	invertMatrix.inverse();
}

Box::Box( const Vector3 &pos , const Vector3 &rot , const Vector3 &size , bool twoSide ) :
	isTwoSide( twoSide ) , center( pos ) , extent( size ) , matrix( rot ) , invertMatrix( rot )
{
	createSurfaces();
	invertMatrix.inverse();
}

Box::Box( const Box &otherBox ) :
	isTwoSide( otherBox.isTwoSide ) , center( otherBox.center ) , extent( otherBox.extent ) , 
	matrix( otherBox.matrix ) , invertMatrix( otherBox.invertMatrix )
{
	if ( this == &otherBox )
		return;
	
	createSurfaces();
}

Box::~Box()
{
	for ( int i = 0; i < 6; ++i )
	{
		delete surfaces[i];
	}
}

void Box::operator=( const Box &otherBox )
{
	for ( int i = 0; i < 6; ++i )
	{
		delete surfaces[i];
	}

	center = otherBox.center;
	extent = otherBox.extent;

	createSurfaces();
}

bool Box::hitTest( const Ray &ray , float t_min , float t_max , HitResult& hitResult )
{
	Vector3 localOrigin = ( ray.getOrigin() - center ) * invertMatrix;
	Vector3 localDir = ray.getDirection() * invertMatrix;
	Ray localRay( localOrigin , localDir , ray.getSendTime() );

	float nearest_t = t_max + 1.0f;
	HitResult tempResult;
	for ( int i = 0; i < 6; ++i )
	{
		if ( surfaces[i]->hitTest( localRay , t_min , t_max , tempResult ) )
		{
			if ( tempResult.t < nearest_t )
			{
				nearest_t = tempResult.t;
				hitResult = tempResult;
			}
		}
	}

	hitResult.hitPoint = hitResult.hitPoint * matrix + center;
	hitResult.hitNormal = hitResult.hitNormal * matrix;
	return nearest_t <= t_max;
}

bool Box::getBoundingBox( float exposureTime , BoundingBox &aabb )
{
	aabb = BoundingBox( center - extent , center + extent );
	return true;
}

void Box::setMaterial( Material *mat )
{
	for ( int i = 0; i < 6; ++i )
	{
		surfaces[i]->setMaterial( mat );
	}
}

void Box::createSurfaces()
{
	Vector3 minPoint = -extent;
	Vector3 maxPoint = extent;

	if ( isTwoSide )
	{
		surfaces[0] = new AxisAlignedRect( EAxis::YZ , ESide::Twoside ,
			minPoint[1] , minPoint[2] , maxPoint[1] , maxPoint[2] , maxPoint[0] );//right
		surfaces[1] = new AxisAlignedRect( EAxis::YZ , ESide::Twoside ,
			minPoint[1] , minPoint[2] , maxPoint[1] , maxPoint[2] , minPoint[0] );//left
		surfaces[2] = new AxisAlignedRect( EAxis::XZ , ESide::Twoside ,
			minPoint[0] , minPoint[2] , maxPoint[0] , maxPoint[2] , maxPoint[1] );//up
		surfaces[3] = new AxisAlignedRect( EAxis::XZ , ESide::Twoside ,
			minPoint[0] , minPoint[2] , maxPoint[0] , maxPoint[2] , minPoint[1] );//down
		surfaces[4] = new AxisAlignedRect( EAxis::XY , ESide::Twoside ,
			minPoint[0] , minPoint[1] , maxPoint[0] , maxPoint[1] , maxPoint[2] );//back
		surfaces[5] = new AxisAlignedRect( EAxis::XY , ESide::Twoside ,
			minPoint[0] , minPoint[1] , maxPoint[0] , maxPoint[1] , minPoint[2] );//front
	}
	else
	{
		surfaces[0] = new AxisAlignedRect( EAxis::YZ , ESide::Frontside ,
			minPoint[1] , minPoint[2] , maxPoint[1] , maxPoint[2] , maxPoint[0] );//right
		surfaces[1] = new AxisAlignedRect( EAxis::YZ , ESide::Backside ,
			minPoint[1] , minPoint[2] , maxPoint[1] , maxPoint[2] , minPoint[0] );//left
		surfaces[2] = new AxisAlignedRect( EAxis::XZ , ESide::Frontside ,
			minPoint[0] , minPoint[2] , maxPoint[0] , maxPoint[2] , maxPoint[1] );//up
		surfaces[3] = new AxisAlignedRect( EAxis::XZ , ESide::Backside ,
			minPoint[0] , minPoint[2] , maxPoint[0] , maxPoint[2] , minPoint[1] );//down
		surfaces[4] = new AxisAlignedRect( EAxis::XY , ESide::Frontside ,
			minPoint[0] , minPoint[1] , maxPoint[0] , maxPoint[1] , maxPoint[2] );//back
		surfaces[5] = new AxisAlignedRect( EAxis::XY , ESide::Backside ,
			minPoint[0] , minPoint[1] , maxPoint[0] , maxPoint[1] , minPoint[2] );//front
	}
}
