#include "PPMImage.h"
#include "Vector3.h"
#include "Ray.h"
#include "Scene.h"
#include "Camera.h"
#include "MyRand.h"

#include <iostream>
#include <time.h>

Vector3 getColor( const Ray &ray , const Scene *world , int depth )
{
	if ( depth >= 100 )
	{
		return Vector3( 1.0f , 1.0f , 1.0f );
	}

	HitResult hitResult;
	if ( world->hitTest( ray , hitResult ) )
	{
		Vector3 nextDir = hitResult.hitNormal + Vector3::getRandomInUnitSphere();
		Ray nextRay( hitResult.hitPoint , nextDir );
		return 0.5f * getColor( nextRay , world , depth + 1 );
	}
	else
	{
		Vector3 dirVec = ray.getDirection();
		dirVec.normalized();
		float y01 = ( dirVec.y() + 1.0f ) * 0.5f;
		return Vector3::lerp( Vector3( 1.0f , 1.0f , 1.0f ) , Vector3( 0.5f , 0.7f , 1.0f ) , y01 );
	}
}

int main()
{
	srand48( time( 0 ) );

	int width = 600;
	int height = 300;
	int subPixel = 100;

	PPMImage image( width , height );
	
	Camera camera( float( width ) / float( height ) , Vector3( 0.0f , 0.0f , 0.0f ) );
	Scene simpleWorld;

	float totalPixel = float( width * height );
	for ( int y = 0; y < height; ++y )
	{
		for ( int x = 0; x < width; ++x )
		{
			Vector3 color;
			for ( int s = 0; s < subPixel; ++s )
			{
				float u = float( x + getRandom01() ) / float( width );
				float v = float( y + getRandom01() ) / float( height );

				color += getColor( camera.getRay( u , v ) , &simpleWorld , 0 );
			}

			color /= float( subPixel );

			int ir = int( color[0] * 255.99f );
			int ig = int( color[1] * 255.99f );
			int ib = int( color[2] * 255.99f );
			image.SetPixel( x , height - y - 1 , ir , ig , ib );
		}
	}

	image.SaveImage( "test8.ppm" );

	system( "Pause" );
	return 0;
}