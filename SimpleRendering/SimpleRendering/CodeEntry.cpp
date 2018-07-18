#define OFFLINE_OR_REALTIME 1
#define screenWidth 800
#define screenHeight 600

#if OFFLINE_OR_REALTIME == 0

#include <iostream>
#include "PPMScreen.h"
#include "Vector3.h"
#include "Camera.h"
#include "Triangle.h"
#include "Box.h"
#include "ShapeRender.h"
#include "DirectionalLight.h"
#include "PPMImage.h"
#include <time.h>

int main()
{
	clock_t timeBase = clock();

	Vector3 bgColor( 0.321f , 0.792f , 0.972f );
	PPMScreen ppmScreen( screenWidth , screenHeight );
	Camera camera( 90.0f , 1.3333f , 1.0f , 500.0f );
	camera.setShapePostion( Vector3( -3.0f , 0.0 , 1.5f ) );
	camera.setShapeRotation( Vector3( 0.0f , 30.0 , 0.0f ) );

	Triangle triangle;
	triangle.setShapePostion( Vector3( 0.0f , 0.0f , 0.0f ) );
	triangle.setShapeRotation( Vector3( 0.0f , -70.0f , 0.0f ) );

	Box box;
	box.setShapePostion( Vector3( 0.0f , 0.5f , 0.0f ) );
	box.setShapeRotation( Vector3( 0.0f , 0.0f , -30.0f ) );

	DirectionalLight mainLight( Vector3( 0.0f , 30.0f , 90.0f ) );

	PPMImage texture( "WoodCrate01.ppm" );

	ShapeRender render( &camera , &ppmScreen );
	render.setCullState( 1 );
	render.setLight( &mainLight );

	clock_t timeInit = clock();

	//////////////////////////SSAA On/////////////////////////
	//clear screen
	ppmScreen.renderBackground( bgColor , Vector3( 1.0f , 1.0f , 1.0f ) );

	clock_t timeRenderBg = clock();

	render.setRenderShape( &box );
	render.setTexture( &texture );
	render.renderShape();

	clock_t timeRenderBox = clock();

	render.setRenderShape( &triangle );
	render.setTexture( nullptr );
	render.renderShape();
	
	clock_t timeRenderTriangle = clock();

	//save image
	ppmScreen.saveImage( "test.ppm" );
	//ppmScreen.saveDepthMap( "testDepth.pgm" );

	clock_t timeSaveImage = clock();

	std::cout << "init time:" << timeInit - timeBase << std::endl;
	std::cout << "background time:" << timeRenderBg - timeInit << std::endl;
	std::cout << "box time:" << timeRenderBox - timeRenderBg << std::endl;
	std::cout << "triangle time:" << timeRenderTriangle - timeRenderBox << std::endl;
	std::cout << "save time:" << timeSaveImage - timeRenderTriangle << std::endl;
	std::cout << "total time:" << timeSaveImage - timeBase << std::endl;

	//////////////////////////SSAA Off/////////////////////////
	//clear screen
	//ppmScreen.setSSAAOnOff( false );
	//ppmScreen.renderBackground( bgColor , Vector3( 1.0f , 1.0f , 1.0f ) );

	//render.setRenderShape( &box );
	//render.renderShape();
	//render.setRenderShape( &triangle );
	//render.renderShape();

	////save image
	//ppmScreen.saveImage( "test1.ppm" );
	
	system( "pause" );
	return 0;
}

#elif OFFLINE_OR_REALTIME == 1
#include "SimpleWindow.h"

int main( int argc , char* argv[] )
{
	SimpleWindow::getInstance().createWindow( &argc , argv , screenWidth , screenHeight );

	return 0;
}


#endif