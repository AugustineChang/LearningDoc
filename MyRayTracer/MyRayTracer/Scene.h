#pragma once
#include "Hitable.h"

class Scene
{
public:
	Scene();
	Scene( float min , float max );
	Scene( const Scene& copy ) = delete;
	~Scene();

	void operator=( const Scene& copy ) = delete;

	bool hitTest( const Ray &ray , HitResult &outResult ) const;

private:

	void createList();

	Hitable **hitableList;
	int listLen;

	float t_min;
	float t_max;
};

