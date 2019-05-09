#include "AxisAlignedRect.h"
#include "BoundingBox.h"
#include "Vector3.h"
#include "Ray.h"

AxisAlignedRect::AxisAlignedRect( EAxis axis , ESide side , float min0 , float min1 , float max0 , float max1 , float k ) :
	alignedAxis( axis ) , faceSide( side )
{
	switch ( alignedAxis )
	{
	case XY:
		minPoint = Vector3( min0 , min1 , k );
		maxPoint = Vector3( max0 , max1 , k );
		break;

	case YZ:
		minPoint = Vector3( k , min0 , min1 );
		maxPoint = Vector3( k , max0 , max1 );
		break;

	case XZ:
		minPoint = Vector3( min0 , k , min1 );
		maxPoint = Vector3( max0 , k , max1 );
		break;
	}
}


bool AxisAlignedRect::hitTest( const Ray &ray , float t_min , float t_max , HitResult& hitResult )
{
	switch ( alignedAxis )
	{
	case XY:
		return hitTest_XY( ray , t_min , t_max , hitResult );

	case YZ:
		return hitTest_YZ( ray , t_min , t_max , hitResult );

	case XZ:
		return hitTest_XZ( ray , t_min , t_max , hitResult );
	}
}

bool AxisAlignedRect::getBoundingBox( float exposureTime , BoundingBox &aabb )
{
	switch ( alignedAxis )
	{
	case XY:
	{
		Vector3 thickness( 0 , 0 , 0.001f );
		aabb = BoundingBox( minPoint - thickness , maxPoint + thickness );
		break;
	}

	case YZ:
	{
		Vector3 thickness( 0.001f , 0 , 0 );
		aabb = BoundingBox( minPoint - thickness , maxPoint + thickness );
		break;
	}

	case XZ:
	{
		Vector3 thickness( 0 , 0.001f , 0 );
		aabb = BoundingBox( minPoint - thickness , maxPoint + thickness );
		break;
	}
	}

	return true;
}

float AxisAlignedRect::sideToFloat( ESide side )
{
	if ( side == ESide::Frontside )
		return 1.0f;
	else if ( side == ESide::Backside )
		return -1.0f;
	else
		return 0.0f;
}

bool AxisAlignedRect::hitTest_XY( const Ray &ray , float t_min , float t_max , HitResult& hitResult )
{
	ESide sideForRay = ray.getOrigin().z() > minPoint.z() ? ESide::Frontside : ESide::Backside;
	if ( faceSide != ESide::Twoside && sideForRay != faceSide )
		return false;

	float hit_t = ( minPoint.z() - ray.getOrigin().z() ) / ray.getDirection().z();
	if ( hit_t < t_min || hit_t > t_max )
		return false;

	float hit_x = ray.getOrigin().x() + hit_t * ray.getDirection().x();
	float hit_y = ray.getOrigin().y() + hit_t * ray.getDirection().y();

	if ( hit_x < minPoint.x() || hit_x > maxPoint.x() || 
		hit_y < minPoint.y() || hit_y > maxPoint.y() )
		return false;
	
	hitResult.t = hit_t;
	hitResult.hitPoint = Vector3( hit_x , hit_y , minPoint.z() );
	hitResult.hitNormal = Vector3::forwardVector * sideToFloat( sideForRay );
	hitResult.mat = objMat;
	hitResult.hitUVCoord[0] = ( hit_x - minPoint.x() ) / ( maxPoint.x() - minPoint.x() );
	hitResult.hitUVCoord[1] = ( hit_y - minPoint.y() ) / ( maxPoint.y() - minPoint.y() );
	return true;
}

bool AxisAlignedRect::hitTest_YZ( const Ray &ray , float t_min , float t_max , HitResult& hitResult )
{
	ESide sideForRay = ray.getOrigin().x() > minPoint.x() ? ESide::Frontside : ESide::Backside;
	if ( faceSide != ESide::Twoside && sideForRay != faceSide )
		return false;

	float hit_t = ( minPoint.x() - ray.getOrigin().x() ) / ray.getDirection().x();
	if ( hit_t < t_min || hit_t > t_max )
		return false;

	float hit_y = ray.getOrigin().y() + hit_t * ray.getDirection().y();
	float hit_z = ray.getOrigin().z() + hit_t * ray.getDirection().z();

	if ( hit_y < minPoint.y() || hit_y > maxPoint.y() ||
		hit_z < minPoint.z() || hit_z > maxPoint.z() )
		return false;
	
	hitResult.t = hit_t;
	hitResult.hitPoint = Vector3( minPoint.x() , hit_y , hit_z );
	hitResult.hitNormal = Vector3::rightVector * sideToFloat( sideForRay );
	hitResult.mat = objMat;
	hitResult.hitUVCoord[0] = ( hit_z - minPoint.z() ) / ( maxPoint.z() - minPoint.z() );
	hitResult.hitUVCoord[1] = ( hit_y - minPoint.y() ) / ( maxPoint.y() - minPoint.y() );
	return true;
}

bool AxisAlignedRect::hitTest_XZ( const Ray &ray , float t_min , float t_max , HitResult& hitResult )
{
	ESide sideForRay = ray.getOrigin().y() > minPoint.y() ? ESide::Frontside : ESide::Backside;
	if ( faceSide != ESide::Twoside && sideForRay != faceSide )
		return false;

	float hit_t = ( minPoint.y() - ray.getOrigin().y() ) / ray.getDirection().y();
	if ( hit_t < t_min || hit_t > t_max )
		return false;

	float hit_x = ray.getOrigin().x() + hit_t * ray.getDirection().x();
	float hit_z = ray.getOrigin().z() + hit_t * ray.getDirection().z();

	if ( hit_x < minPoint.x() || hit_x > maxPoint.x() ||
		hit_z < minPoint.z() || hit_z > maxPoint.z() )
		return false;
	
	hitResult.t = hit_t;
	hitResult.hitPoint = Vector3( hit_x , minPoint.y() , hit_z );
	hitResult.hitNormal = Vector3::upVector * sideToFloat( sideForRay );
	hitResult.mat = objMat;
	hitResult.hitUVCoord[0] = ( hit_x - minPoint.x() ) / ( maxPoint.x() - minPoint.x() );
	hitResult.hitUVCoord[1] = ( hit_z - minPoint.z() ) / ( maxPoint.z() - minPoint.z() );
	return true;
}
