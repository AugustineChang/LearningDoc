#include "PPMImage.h"
#include "Vector3.h"
#include "Scene.h"
#include "Camera.h"
#include "DrawTask.h"
#include "MyMath.h"

#include <string>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <time.h>

#include <thread>
#include <mutex>

void doPrintData( float *progress , int numOfThreads )
{
	std::cout << std::setprecision( 3 );

	while ( true )
	{
		system( "cls" );

		int finishNum = 0;
		for ( int i = 0; i < numOfThreads; ++i )
		{
			if ( progress[i] >= 1.0f )
			{
				std::cout << "绘制进度：" << "100.000%" << std::endl;
				++finishNum;
			}
			else
			{
				std::cout << "绘制进度：" << progress[i] * 100.0f << '%' << std::endl;
			}
		}

		if ( finishNum >= numOfThreads )
		{
			std::cout << "绘制完成！" << std::endl;
			break;
		}
		else
		{
			std::chrono::milliseconds sleepTime( 100 );
			std::this_thread::sleep_for( sleepTime );
		}
	}
}

#define SCENE_NUM 3

int main()
{
	MyMath::srand48( static_cast<unsigned int>( time( 0 ) ) );

	int width = 600;
	int height = 400;
	int subPixel = 100;
	const int numOfThreads = 4;

	//exclusive Data
	float drawProgress[numOfThreads] = { 0.0f };
	
	//shared Data
	
	PPMImage image( width , height );
	Scene simpleWorld;
	
#if SCENE_NUM == 0
	std::string outputName( "MotionBlur.ppm" );

	Camera camera( Vector3( 13.0f , 2.0f , 3.0f ) , Vector3( 0.0f , 0.0f , 0.0f ) ,
		float( width ) / float( height ) , 20.0f , 0.1f , 10.0f , 1.0f );

	simpleWorld.randomScene();
	simpleWorld.createBVT( camera.getExposureTime() );
#elif SCENE_NUM == 1
	std::string outputName( "TestPerlin.ppm" );

	Camera camera( Vector3( 0.0f , 2.0f , 6.0f ) , Vector3( 0.0f , 0.0f , 0.0f ) ,
		float( width ) / float( height ) , 20.0f , 0.0f , 10.0f , 1.0f );

	simpleWorld.createTestScene();
#elif SCENE_NUM == 2
	std::string outputName( "DarkWorld.ppm" );

	Camera camera( Vector3( 0.0f , 2.0f , 6.0f ) , Vector3( 0.0f , 0.0f , 0.0f ) ,
		float( width ) / float( height ) , 20.0f , 0.0f , 10.0f , 1.0f );

	simpleWorld.createDarkScene();
#elif SCENE_NUM == 3
	subPixel = 200;
	std::string outputName( "CornellBox.ppm" );

	Camera camera( Vector3( 0.0f , 0.0f , 5.0f ) , Vector3( 0.0f , 0.0f , 0.0f ) ,
		float( width ) / float( height ) , 30.0f , 0.0f , 10.0f , 0.0f );

	simpleWorld.createCornellBox();
#endif

	//start printThread
	std::thread printThread( doPrintData , drawProgress , numOfThreads );
	
	//start 4 threads to draw
	std::mutex mtx;
	int eachHeight = height / numOfThreads;
	if ( height % numOfThreads > 0 ) eachHeight += 1;

	for ( int i = 0; i < numOfThreads; ++i )
	{
		int startRow = i * eachHeight;
		int endRow = ( i == numOfThreads - 1 ) ? height - 1 : ( i + 1 ) * eachHeight - 1;

		DrawTask task( startRow , endRow , width , height , subPixel , &mtx , &image , &simpleWorld ,
			&camera , &drawProgress[i] );

		std::thread drawThread( task );
		drawThread.detach();
	}

	//wait printComplete
	printThread.join();

	//save to image
	image.SaveImage( outputName );

	system( "Pause" );
	return 0;
}