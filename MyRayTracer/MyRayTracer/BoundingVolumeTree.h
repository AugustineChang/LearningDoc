#pragma once
#include "Hitable.h"
#include "BoundingBox.h"

class BoundingVolumeTree;
typedef bool ( BoundingVolumeTree::*CompareFunc )( Hitable *a , Hitable *b );

class BoundingVolumeTree : public Hitable
{
public:

	BoundingVolumeTree();
	BoundingVolumeTree( Hitable ** objList , int numOfList , float exposureTime );

public:

	virtual bool hitTest( const Ray &ray , float t_min , float t_max , HitResult& hitResult ) override;
	virtual bool getBoundingBox( float exposureTime , BoundingBox &aabb ) override;

	void sort( Hitable **objList , int numOfList , CompareFunc compFunc );

private:

	void merge( Hitable **objList , int start , int mid , int end , Hitable **tempList , CompareFunc func );
	void mergeSort( Hitable **objList , int start , int end , Hitable **tempList , CompareFunc func );

	bool compareAxisX( Hitable *objA , Hitable *objB );
	bool compareAxisY( Hitable *objA , Hitable *objB );
	bool compareAxisZ( Hitable *objA , Hitable *objB );
	bool compareAxis( Hitable *objA , Hitable *objB , int axis );

private:

	Hitable *leftTree;
	Hitable *rightTree;

	BoundingBox cachedBox;
};

