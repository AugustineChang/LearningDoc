#include "PPMImage.h"
#include "Vector3.h"
#include "Ray.h"
#include "Scene.h"
#include "Camera.h"
#include "MyRand.h"
#include "Material.h"

#include <iostream>
#include <time.h>

Vector3 getColor( const Ray &ray , const Scene *world , int depth )
{
	if ( depth >= 50 )
	{
		return Vector3( 1.0f , 1.0f , 1.0f );
	}

	HitResult hitResult;
	if ( world->hitTest( ray , hitResult ) )
	{
		if ( hitResult.mat == nullptr )
		{
			return Vector3( 1.0f , 0.0f , 1.0f );
		}
		else
		{
			Vector3 attenuation;
			Ray nextRay;

			if ( hitResult.mat->scatter( ray , hitResult , attenuation , nextRay ) )
			{
				return attenuation * getColor( nextRay , world , depth + 1 );
			}
			else
				return Vector3( 0.0f , 0.0f , 0.0f );
		}
		
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
	srand48( static_cast<unsigned int>( time( 0 ) ) );

	int width = 1200;
	int height = 800;
	int subPixel = 10;

	PPMImage image( width , height );
	
	Camera camera( Vector3( 13.0f , 2.0f , 3.0f ) , Vector3( 0.0f , 0.0f , 0.0f ) ,
		float( width ) / float( height ) , 20.0f , 0.1f , 10.0f );
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

	image.SaveImage( "test12.ppm" );

	system( "Pause" );
	return 0;
}