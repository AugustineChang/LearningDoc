#include "Scene.h"
#include "Sphere.h"


Scene::Scene() : t_min( 0.001f ) , t_max( 1000.0f ) , hitableList( nullptr )
{
	createList();
}


Scene::Scene( float min , float max ) : t_min( min ) , t_max( max ) , hitableList( nullptr )
{
	createList();
}

Scene::~Scene()
{
	for ( int i = 0; i < listLen; ++i )
	{
		delete hitableList[i];
	}
	delete[] hitableList;
}

bool Scene::hitTest( const Ray &ray , HitResult &outResult ) const
{
	HitResult tempResult;
	float t_real = t_max;

	bool isHit = false;
	for ( int i = 0; i < listLen; ++i )
	{
		if ( hitableList[i]->hitTest( ray , t_min , t_real , tempResult ) )
		{
			t_real = tempResult.t;
			isHit = true;
			outResult = tempResult;
		}
	}
	
	return isHit;
}

void Scene::createList()
{
	listLen = 2;
	hitableList = new Hitable* [listLen];
	hitableList[0] = new Sphere( Vector3( 0.0f , 0.0f , -1.0f ) , 0.5f );
	hitableList[1] = new Sphere( Vector3( 0.0f , -100.5f , -1.0f ) , 100.0f );
}
