#include "Scene.h"
#include "Sphere.h"
#include "Lanbertain.h"
#include "Metal.h"
#include "Glass.h"
#include "MyRand.h"

Scene::Scene() : t_min( 0.001f ) , t_max( 1000.0f ) , hitableList( nullptr ) , matList( nullptr )
{
	randomScene();
}


Scene::Scene( float min , float max ) : t_min( min ) , t_max( max ) , hitableList( nullptr ) , matList( nullptr )
{
	randomScene();
}

Scene::~Scene()
{
	for ( int i = 0; i < hitableNum; ++i )
	{
		delete hitableList[i];
	}
	delete[] hitableList;

	for ( int i = 0; i < matNum; ++i )
	{
		delete matList[i];
	}
	delete[] matList;
}

bool Scene::hitTest( const Ray &ray , HitResult &outResult ) const
{
	HitResult tempResult;
	float t_real = t_max;

	bool isHit = false;
	for ( int i = 0; i < hitableNum; ++i )
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

void Scene::createObjList()
{
	hitableNum = 4;
	hitableList = new Hitable* [hitableNum];
	hitableList[0] = new Sphere( Vector3( 0.8f , 0.0f , -1.0f ) , 0.3f );
	hitableList[1] = new Sphere( Vector3( 0.0f , 0.0f , -1.0f ) , 0.5f );
	hitableList[2] = new Sphere( Vector3( -0.8f , 0.0f , -1.0f ) , 0.3f );
	hitableList[3] = new Sphere( Vector3( 0.0f , -100.5f , -1.0f ) , 100.0f );

	hitableList[0]->setMaterial( matList[1] );
	hitableList[1]->setMaterial( matList[0] );
	hitableList[2]->setMaterial( matList[2] );
	hitableList[3]->setMaterial( matList[3] );
}

void Scene::createMaterials()
{
	matNum = 4;
	matList = new Material *[matNum];
	matList[0] = new Lambertain( Vector3( 0.0f , 0.5f , 1.0f ) );
	matList[1] = new Metal( Vector3( 1.0f , 1.0f , 1.0f ) , 0.3f );
	matList[2] = new Glass( Vector3( 1.0f , 1.0f , 1.0f ), 1.5f );
	matList[3] = new Lambertain( Vector3( 0.4f , 0.4f , 0.4f ) );
}

void Scene::randomScene()
{
	hitableNum = 501;
	matNum = hitableNum;

	hitableList = new Hitable*[hitableNum];
	matList = new Material*[matNum];

	hitableList[0] = new Sphere( Vector3( 0.0f , -1000.0f , 0.0f ) , 1000.0f );
	matList[0] = new Lambertain( Vector3( 0.5f , 0.5f , 0.5f ) );

	int index = 1;

	for ( int xPos = -11; xPos < 11; ++xPos )
	{
		for ( int yPos = -11; yPos < 11; ++yPos )
		{
			Vector3 center = Vector3( xPos + getRandom01() * 0.9f , 0.2f , yPos + getRandom01() * 0.9f );
			if ( ( center - Vector3( 4.0f , 0.2f , 0.0f ) ).length() <= 0.9f ) continue;
			
			hitableList[index] = new Sphere( center , 0.2f );
			
			float chooseMat = getRandom01();
			if ( chooseMat < 0.8f )//diffuse
			{
				matList[index] = new Lambertain( Vector3::getRandomColor() * Vector3::getRandomColor() );
			}
			else if ( chooseMat < 0.95f )//metal
			{
				Vector3 color = ( Vector3::getRandomColor() + Vector3( 1.0f , 1.0f , 1.0f ) )*0.5f;
				matList[index] = new Metal( color , 0.5f * getRandom01() );
			}
			else//glass
			{
				matList[index] = new Glass( 1.5f );
			}

			++index;
		}
	}

	hitableList[index] = new Sphere( Vector3( 0.0f , 1.0f , 0.0f ) , 1.0f );
	matList[index] = new Glass( 1.5f );
	++index;

	hitableList[index] = new Sphere( Vector3( -4.0f , 1.0f , 0.0f ) , 1.0f );
	matList[index] = new Lambertain( Vector3( 0.4f , 0.2f , 0.1f ) );
	++index;

	hitableList[index] = new Sphere( Vector3( 4.0f , 1.0f , 0.0f ) , 1.0f );
	matList[index] = new Metal( Vector3( 0.7f , 0.6f , 0.5f ) , 0.0f );
	++index;

	hitableNum = index;
	matNum = index;

	for ( int i = 0; i < hitableNum; ++i )
	{
		hitableList[i]->setMaterial( matList[i] );
	}
}
