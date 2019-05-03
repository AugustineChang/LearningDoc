#include "Scene.h"
#include "MyMath.h"
#include "BoundingVolumeTree.h"
//shape
#include "Sphere.h"
//texture
#include "ConstTexture.h"
#include "GridTexture.h"
#include "PerlinTexture.h"
//material
#include "Lanbertain.h"
#include "Metal.h"
#include "Glass.h"

Scene::Scene() : 
	bvTree( nullptr ) ,
	hitableList( nullptr ) , 
	matList( nullptr ) ,
	texList( nullptr ) ,
	hitableNum( 0 ) ,
	matNum( 0 ) ,
	texNum( 0 ) ,
	t_min( 0.001f ) , 
	t_max( 1000.0f )
{
}


Scene::Scene( float min , float max ) : 
	bvTree( nullptr ) ,
	hitableList( nullptr ) ,
	matList( nullptr ) ,
	texList( nullptr ) ,
	hitableNum( 0 ) ,
	matNum( 0 ) ,
	texNum( 0 ) ,
	t_min( min ) ,
	t_max( max )
{
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

	for ( int i = 0; i < texNum; ++i )
	{
		delete texList[i];
	}
	delete[] texList;

	if ( bvTree != nullptr )
	{
		delete bvTree;
	}
}

void Scene::createBVT( float exposureTime )
{
	if ( hitableList == nullptr || hitableNum <= 0 )
		return;

	bvTree = new BoundingVolumeTree( hitableList , hitableNum , exposureTime );
}

bool Scene::hitTest( const Ray &ray , HitResult &outResult ) const
{
	if ( bvTree != nullptr )
	{
		return bvTree->hitTest( ray , t_min , t_max , outResult );
	}
	else
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
}

void Scene::createObjList()
{
	hitableNum = 4;
	matNum = 4;
	texNum = 4;

	hitableList = new Hitable* [hitableNum];
	hitableList[0] = new Sphere( Vector3( 0.8f , 0.0f , -1.0f ) , 0.3f );
	hitableList[1] = new Sphere( Vector3( 0.0f , 0.0f , -1.0f ) , 0.5f );
	hitableList[2] = new Sphere( Vector3( -0.8f , 0.0f , -1.0f ) , 0.3f );
	hitableList[3] = new Sphere( Vector3( 0.0f , -100.5f , -1.0f ) , 100.0f );

	texList = new Texture *[texNum];
	texList[0] = new ConstTexture( Vector3( 0.0f , 0.5f , 1.0f ) );
	texList[1] = new ConstTexture( Vector3( 1.0f , 1.0f , 1.0f ) );
	texList[2] = new ConstTexture( Vector3( 0.4f , 0.4f , 0.4f ) );
	texList[3] = new PerlinTexture();

	matList = new Material *[matNum];
	matList[0] = new Lambertain( texList[0] );
	matList[1] = new Metal( texList[1] , 0.3f );
	matList[2] = new Glass( texList[1] , 1.5f );
	matList[3] = new Lambertain( texList[3] );

	hitableList[0]->setMaterial( matList[1] );
	hitableList[1]->setMaterial( matList[0] );
	hitableList[2]->setMaterial( matList[2] );
	hitableList[3]->setMaterial( matList[3] );
}

void Scene::randomScene()
{
	hitableNum = 501;
	matNum = hitableNum;
	texNum = hitableNum;

	hitableList = new Hitable*[hitableNum];
	texList = new Texture*[texNum];
	matList = new Material*[matNum];

	hitableList[0] = new Sphere( Vector3( 0.0f , -1000.0f , 0.0f ) , 1000.0f );
	//texList[0] = new ConstTexture( Vector3( 0.5f , 0.5f , 0.5f ) );
	texList[0] = new GridTexture( 0.5f , Vector3::oneVector , Vector3( 0.0f , 0.383f , 0.449f ) );
	matList[0] = new Lambertain( texList[0] );

	int index = 1;
	int texIndex = 1;

	for ( int xPos = -11; xPos < 11; ++xPos )
	{
		for ( int yPos = -11; yPos < 11; ++yPos )
		{
			Vector3 center = Vector3( xPos + MyMath::getRandom01() * 0.9f , 0.2f , yPos + MyMath::getRandom01() * 0.9f );
			if ( ( center - Vector3( 4.0f , 0.2f , 0.0f ) ).length() <= 0.9f ) continue;
			
			float moveable = MyMath::getRandom01();
			hitableList[index] = new Sphere( center , 0.2f , moveable >= 0.5f );
			
			float chooseMat = MyMath::getRandom01();
			if ( chooseMat < 0.8f )//diffuse
			{
				Vector3 color = Vector3::getRandomColor() * Vector3::getRandomColor();
				texList[texIndex] = new ConstTexture( color );
				matList[index] = new Lambertain( texList[texIndex] );

				++texIndex;
			}
			else if ( chooseMat < 0.95f )//metal
			{
				Vector3 color = ( Vector3::getRandomColor() + Vector3( 1.0f , 1.0f , 1.0f ) )*0.5f;
				texList[texIndex] = new ConstTexture( color );
				matList[index] = new Metal( texList[texIndex] , 0.5f * MyMath::getRandom01() );

				++texIndex;
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
	texList[texIndex] = new ConstTexture( Vector3( 0.4f , 0.2f , 0.1f ) );
	matList[index] = new Lambertain( texList[texIndex] );
	++index;
	++texIndex;

	hitableList[index] = new Sphere( Vector3( 4.0f , 1.0f , 0.0f ) , 1.0f );
	texList[texIndex] = new ConstTexture( Vector3( 0.7f , 0.6f , 0.5f ) );
	matList[index] = new Metal( texList[texIndex] , 0.0f );
	++index;
	++texIndex;

	hitableNum = index;
	matNum = index;
	texNum = texIndex;

	for ( int i = 0; i < hitableNum; ++i )
	{
		hitableList[i]->setMaterial( matList[i] );
	}
}
