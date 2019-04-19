#pragma once
#include "Hitable.h"

class BoundingBox : public Hitable
{
public:
	BoundingBox();
	BoundingBox( const Vector3 &min , const Vector3 &max );
	BoundingBox( const BoundingBox &boxA , const BoundingBox &boxB );

public:

	virtual bool hitTest( const Ray &ray , float t_min , float t_max , HitResult& hitResult ) override;
	virtual bool getBoundingBox( float exposureTime , BoundingBox &aabb ) override;

	bool isEmpty() const;
	Vector3 GetMin() const;
	Vector3 GetCenter() const;
	Vector3 GetMax() const;

private:

	void swap( float &a , float &b );

	Vector3 boxMin;
	Vector3 boxMax;
};

