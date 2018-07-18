#include "SimpleWindow.h"
#include <GL/glut.h>
#include <iostream>

#include "Vector3.h"
#include "Triangle.h"
#include "Box.h"
#include "ShapeRender.h"
#include "PPMImage.h"
#include "ScreenBuffer.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "SimpleMath.h"

#define GLUT_WHEEL_UP 3
#define GLUT_WHEEL_DOWN 4

SimpleWindow::SimpleWindow() : isCameraRot( false ) , isDrawBg( true ) , cameraYaw( 0.0f ) , cameraPitch( 0.0f ) ,
	cameraMoveForward( 0.0f ) , cameraMoveRight( 0.0f ) , cameraRadius( 5.0f ) , cameraX( 0.5f ) , cameraY( -7.0f )
{
}

SimpleWindow::~SimpleWindow()
{
}

SimpleWindow& SimpleWindow::getInstance()
{
	static SimpleWindow window;
	return window;
}

void SimpleWindow::createWindow( int *argc , char**argv , int w , int h )
{
	windowWidth = w;
	windowHeight = h;

	glutInit( argc , argv );
	glutInitDisplayMode( GLUT_RGB | GLUT_SINGLE );
	glutInitWindowPosition( 100 , 100 );
	glutInitWindowSize( windowWidth , windowHeight );
	glutCreateWindow( "SimpleRender" );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0 , windowWidth , windowHeight , 0 );
	glViewport( 0 , 0 , windowWidth , windowHeight );

	initScene();

	glutDisplayFunc( globalDraw );
	glutMouseFunc( globalMouseEvent );
	glutMotionFunc( globalMouseMove );
	glutIdleFunc( globalMainLoop );
	glutKeyboardFunc( globalKeyboardEvent );
	glutKeyboardUpFunc( globalKeyboardUpEvent );
	glutMainLoop();
}

void SimpleWindow::gameMainLoop()
{
	gameTimer.Tick();
	gameTimer.CalcFrameStat();
	updateScene( gameTimer.DeltaTime() );
	glutPostRedisplay();
}

void SimpleWindow::initScene()
{
	gameTimer.Reset();

	screenBuffer = new ScreenBuffer( windowWidth , windowHeight );
	camera = new Camera( 90.0f , 1.3333f , 1.0f , 500.0f );
	camera->setShapePostion( Vector3( -5.0f , 0.0f , 0.0f ) );
	camera->setShapeRotation( Vector3( 0.0f , 0.0f , 0.0f ) );

	triangle = new Triangle();
	triangle->setShapePostion( Vector3( 0.0f , 0.0f , 0.0f ) );
	triangle->setShapeRotation( Vector3( 0.0f , -70.0f , 0.0f ) );

	box = new Box();
	box->setShapePostion( Vector3( 0.0f , 0.0f , 0.0f ) );

	mainLight = new DirectionalLight( Vector3( 0.0f , 0.86666f , -0.5f ) );
	texture = new PPMImage( "WoodCrate01.ppm" );

	render = new ShapeRender( camera , screenBuffer );
	render->setCullState( 1 );
	render->setLight( mainLight );
}

void SimpleWindow::mouseEvent( int button , int state , int x , int y )
{
	switch ( button )
	{
	case GLUT_RIGHT_BUTTON:
		if ( state == GLUT_DOWN )
		{
			isCameraRot = true;

			lastMouseX = x;
			lastMouseY = y;
		}
		else if ( state == GLUT_UP )
		{
			isCameraRot = false;
		}
		break;

	default:
		break;
	}
}

void SimpleWindow::mouseMove( int x , int y )
{
	/*if ( isCameraRot )
	{
		cameraYaw = float( x - lastMouseX ) / windowWidth * 50.0f;
		cameraPitch = float( y - lastMouseY ) / windowHeight * 50.0f;

		lastMouseX = x;
		lastMouseY = y;
	}*/

	if ( isCameraRot )
	{
		cameraYaw = float( y - lastMouseY ) / windowHeight * 5.0f;
		cameraPitch = float( x - lastMouseX ) / windowWidth * 5.0f;

		lastMouseX = x;
		lastMouseY = y;
	}
}

void SimpleWindow::keyboardEvent( unsigned char key , bool isUp )
{
	switch ( key )
	{
	case '1':
		if ( isUp )
		{
			box->setDrawMode( 0 );
			triangle->setDrawMode( 0 );
			isDrawBg = false;
		}
		break;

	case '2':
		if ( isUp )
		{
			box->setDrawMode( 1 );
			triangle->setDrawMode( 1 );
			isDrawBg = true;
		}
		break;

	case '3':
		if ( isUp )
		{
			isShowBoxTexture = !isShowBoxTexture;
		}
		break;

	case 'w':
		if ( !isUp )
			cameraMoveForward = 1.0f;
		else if ( cameraMoveForward > 0.0f )
			cameraMoveForward = 0.0f;
		break;

	case 's':
		if ( !isUp )
			cameraMoveForward = -1.0f;
		else if ( cameraMoveForward < 0.0f )
			cameraMoveForward = 0.0f;
		break;

	case 'a':
		if ( !isUp )
			cameraMoveRight = -1.0f;
		else if ( cameraMoveRight < 0.0f )
			cameraMoveRight = 0.0f;
		break;

	case 'd':
		if ( !isUp )
			cameraMoveRight = 1.0f;
		else if ( cameraMoveRight > 0.0f )
			cameraMoveRight = 0.0f;
		break;

	case '=':
		if ( isCameraRot )
		{
			cameraRadius += 0.5f;
		}
		break;

	case '-':
		if ( isCameraRot )
		{
			cameraRadius -= 0.5f;
		}
		break;

	default:
		break;
	}
}

void SimpleWindow::updateScene( float deltaTime )
{
	//box->addShapeRotation( Vector3( 0.0f , 0.0f , 10.0f * deltaTime ) );

	//cameraMove( deltaTime );
	cameraMoveOnSphere( deltaTime );
}

void SimpleWindow::cameraMove( float deltaTime )
{
	if ( cameraMoveForward != 0.0f )
	{
		camera->addShapePostion( camera->getForward() * cameraMoveForward * deltaTime );
	}

	if ( cameraMoveRight != 0.0f )
	{
		camera->addShapePostion( camera->getRight() * cameraMoveRight * deltaTime );
	}

	if ( isCameraRot )
	{
		camera->addShapeRotation( Vector3( 0.0f , cameraPitch , cameraYaw ) );
		cameraPitch = 0.0f;
		cameraYaw = 0.0f;
	}
}

void SimpleWindow::cameraMoveOnSphere( float deltaTime )
{
	if ( isCameraRot )
	{
		cameraX += cameraPitch;
		cameraY += cameraYaw;

		cameraPitch = 0.0f;
		cameraYaw = 0.0f;

		float sinY = sinf( cameraY ) , cosY = cosf( cameraY );
		float sinP = sinf( cameraX ) , cosP = cosf( cameraX );

		Vector3 camPos;
		camPos[0] = cameraRadius * sinY * cosP;
		camPos[1] = cameraRadius * sinY * sinP;
		camPos[2] = cameraRadius * cosY;

		camera->setShapePostion( camPos );
		camera->rotateCameraByLookAt( Vector3() , sinY < 0.0f ? Vector3( 0 , 0 , 1 ) : Vector3( 0 , 0 , -1 ) );
	}
}

void SimpleWindow::drawScene()
{
	//gameTimer.beginDebugTime();
	if ( isDrawBg )
		screenBuffer->renderBackground( 82 , 202 , 248 );
	else
		screenBuffer->clearImage();

	render->setRenderShape( box );
	if ( isShowBoxTexture )
		render->setTexture( texture );
	render->renderShape();
	render->setRenderShape( triangle );
	render->setTexture( nullptr );
	render->renderShape();

	//gameTimer.endDebugTime();
	//gameTimer.showDebugTime( "myRender draw:" );

	//gameTimer.beginDebugTime();

	glRasterPos2i( 0 , windowHeight );
	glDrawPixels( windowWidth , windowHeight , GL_RGB , GL_UNSIGNED_BYTE , screenBuffer->getColorBuffer() );

	glFlush();

	//gameTimer.endDebugTime();
	//gameTimer.showDebugTime( "opengl draw:" );
}

void globalDraw()
{
	SimpleWindow::getInstance().drawScene();
}

void globalMainLoop()
{
	SimpleWindow::getInstance().gameMainLoop();
}

void globalMouseEvent( int button , int state , int x , int y )
{
	SimpleWindow::getInstance().mouseEvent( button , state , x , y );
}

void globalMouseMove( int x , int y )
{
	SimpleWindow::getInstance().mouseMove( x , y );
}

void globalKeyboardEvent( unsigned char key , int x , int y )
{
	SimpleWindow::getInstance().keyboardEvent( key , false );
}

void globalKeyboardUpEvent( unsigned char key , int x , int y )
{
	SimpleWindow::getInstance().keyboardEvent( key , true );
}
