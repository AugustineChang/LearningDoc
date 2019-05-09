#include "DrawTask.h"
#include "PPMImage.h"
#include "Vector3.h"
#include "Ray.h"
#include "Scene.h"
#include "Camera.h"
#include "MyMath.h"
#include "Material.h"

DrawTask::DrawTask( int from , int to , int w , int h , int subP , std::mutex *mtx ,
	PPMImage *img , const Scene *world , const Camera *cam , float *prog )
	:fromRow( from ) , toRow( to ) , width( w ) , height( h ) , subPixel( subP ) , mtx( mtx ) ,
	image( img ) , simpleWorld( world ) , camera( cam ) , drawProgress( prog )
{
}

void DrawTask::operator()()
{
	float totalPixel = float( width * ( toRow - fromRow + 1 ) );
	for ( int y = fromRow; y <= toRow; ++y )
	{
		for ( int x = 0; x < width; ++x )
		{
			Vector3 color;
			for ( int s = 0; s < subPixel; ++s )
			{
				float u = float( x + MyMath::getRandom01() ) / float( width );
				float v = float( y + MyMath::getRandom01() ) / float( height );

				color += getColor( camera->getRay( u , v ) , 0 );
			}

			color /= float( subPixel );
			//TODO: color may > 1, need tonemapper

			//gamma correct: pow(color, 0.45)
			color[0] = MyMath::power( color[0] , 0.45f );
			color[1] = MyMath::power( color[1] , 0.45f );
			color[2] = MyMath::power( color[2] , 0.45f );

			//no tonemapper, so simple clamp
			color[0] = MyMath::clamp01( color[0] );
			color[1] = MyMath::clamp01( color[1] );
			color[2] = MyMath::clamp01( color[2] );

			int ir = int( color[0] * 255.99f );
			int ig = int( color[1] * 255.99f );
			int ib = int( color[2] * 255.99f );

			mtx->lock();
			image->SetPixel( x , height - y - 1 , ir , ig , ib );
			mtx->unlock();

			*drawProgress = float( x + ( y - fromRow ) * width + 1 ) / totalPixel;
		}
	}
}

Vector3 DrawTask::getColor( const Ray &ray , int depth )
{
	if ( depth >= 50 )
	{
		return Vector3( 1.0f , 1.0f , 1.0f );
	}

	HitResult hitResult;
	if ( simpleWorld->hitTest( ray , hitResult ) )
	{
		if ( hitResult.mat == nullptr )
		{
			return Vector3( 1.0f , 0.0f , 1.0f );
		}
		else
		{
			Vector3 attenuation;
			Ray nextRay;

			Vector3 emitLight = hitResult.mat->emitted( hitResult );
			if ( hitResult.mat->scatter( ray , hitResult , attenuation , nextRay ) )
				return emitLight + attenuation * getColor( nextRay , depth + 1 );
			else
				return emitLight;
		}

	}
	else
	{
		if ( simpleWorld->isUseSkyLight() )
		{
			Vector3 dirVec = ray.getDirection();
			dirVec.normalized();
			float y01 = ( dirVec.y() + 1.0f ) * 0.5f;
			return Vector3::lerp( Vector3( 1.0f , 1.0f , 1.0f ) , Vector3( 0.5f , 0.7f , 1.0f ) , y01 );
		}
		else
		{
			return Vector3::zeroVector;
		}
	}
}
