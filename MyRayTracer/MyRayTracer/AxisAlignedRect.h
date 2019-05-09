#pragma once
#include "Hitable.h"
#include "Vector3.h"

enum EAxis
{
	XY,
	XZ,
	YZ
};

enum ESide
{
	Frontside ,
	Backside ,
	Twoside
};

class AxisAlignedRect : public Hitable
{
public:

	AxisAlignedRect( EAxis axis , ESide side , float min0 , float min1, float max0, float max1 , float k );
	
	virtual bool hitTest( const Ray &ray , float t_min , float t_max , HitResult& hitResult ) override;
	virtual bool getBoundingBox( float exposureTime , BoundingBox &aabb ) override;

private:

	float sideToFloat( ESide side );

	bool hitTest_XY( const Ray &ray , float t_min , float t_max , HitResult& hitResult );
	bool hitTest_YZ( const Ray &ray , float t_min , float t_max , HitResult& hitResult );
	bool hitTest_XZ( const Ray &ray , float t_min , float t_max , HitResult& hitResult );

	EAxis alignedAxis;
	Vector3 minPoint;
	Vector3 maxPoint;
	ESide faceSide;
};

