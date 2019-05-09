#pragma once
#include "Hitable.h"

class BoundingVolumeTree;
class Texture;

class Scene
{
public:
	Scene();
	Scene( float min , float max );
	Scene( const Scene& copy ) = delete;
	~Scene();

	void operator=( const Scene& copy ) = delete;

	void createBVT( float exposureTime );
	bool hitTest( const Ray &ray , HitResult &outResult ) const;

	bool isUseSkyLight() const
	{
		return useSkyLight;
	}

public:

	void createTestScene();
	void createDarkScene();
	void createCornellBox();
	void randomScene();

private:

	BoundingVolumeTree *bvTree;
	Hitable **hitableList;
	Material **matList;
	Texture **texList;
	int hitableNum;
	int matNum;
	int texNum;

	float t_min;
	float t_max;

	bool useSkyLight;
};

