#pragma once
#include "Hitable.h"
#include "Box.h"

class VolumeFog : public Hitable
{
public:
	VolumeFog( const Vector3 &center , const Vector3 &rot , const Vector3 &extent , float density );
	
	virtual bool hitTest( const Ray &ray , float t_min , float t_max , HitResult& hitResult ) override;
	virtual bool getBoundingBox( float exposureTime , BoundingBox &aabb ) override;

private:

	Box boundary;
	float fogDensity;
};

