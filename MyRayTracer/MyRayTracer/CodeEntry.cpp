#include "PPMImage.h"
#include "Vector3.h"
#include "Scene.h"
#include "Camera.h"
#include "DrawTask.h"
#include "MyMath.h"

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
	Camera camera( Vector3( 13.0f , 2.0f , 3.0f ) , Vector3( 0.0f , 0.0f , 0.0f ) ,
		float( width ) / float( height ) , 20.0f , 0.1f , 10.0f , 1.0f );
	Scene simpleWorld;
	simpleWorld.createBVT( camera.getExposureTime() );

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
	image.SaveImage( "MotionBlur.ppm" );

	system( "Pause" );
	return 0;
}