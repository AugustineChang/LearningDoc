#pragma once
#include "Hitable.h"
#include "RotateMatrix.h"

class Box : public Hitable
{
public:
	Box( const Vector3 &pos );
	Box( const Vector3 &pos , const Vector3 &rot );
	Box( const Vector3 &pos , const Vector3 &rot , const Vector3 &size );
	Box( const Box &otherBox );
	~Box();
	
	void operator=( const Box &otherBox );

	virtual bool hitTest( const Ray &ray , float t_min , float t_max , HitResult& hitResult ) override;
	virtual bool getBoundingBox( float exposureTime , BoundingBox &aabb ) override;
	virtual void setMaterial( Material *mat ) override;

private:

	void createSurfaces();

	Vector3 center;
	Vector3 extent;
	RotateMatrix matrix;
	RotateMatrix invertMatrix;
	Hitable *surfaces[6];
};

