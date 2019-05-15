#pragma once
#include "Hitable.h"
#include "RotateMatrix.h"

class Box : public Hitable
{
public:
	Box( const Vector3 &pos , bool twoSide = false );
	Box( const Vector3 &pos , const Vector3 &rot , bool twoSide = false );
	Box( const Vector3 &pos , const Vector3 &rot , const Vector3 &size , bool twoSide = false );
	Box( const Box &otherBox );
	~Box();
	
	void operator=( const Box &otherBox );

	virtual bool hitTest( const Ray &ray , float t_min , float t_max , HitResult& hitResult ) override;
	virtual bool getBoundingBox( float exposureTime , BoundingBox &aabb ) override;
	virtual void setMaterial( Material *mat ) override;

private:

	void createSurfaces();

	bool isTwoSide;
	Vector3 center;
	Vector3 extent;
	RotateMatrix matrix;
	RotateMatrix invertMatrix;
	Hitable *surfaces[6];
};

