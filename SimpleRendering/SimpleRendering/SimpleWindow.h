#pragma once
#include "GameTimer.h"

class SimpleWindow
{
public:
	SimpleWindow();
	SimpleWindow( const SimpleWindow &copy ) = delete;
	void operator=( const SimpleWindow &copy ) = delete;
	~SimpleWindow();

	static SimpleWindow& getInstance();
	void createWindow( int *argc , char**argv , int w , int h );

	void gameMainLoop();
	void drawScene();

	void mouseEvent( int button , int state , int x , int y );
	void mouseMove( int x , int y );

	void keyboardEvent( unsigned char key , bool isUp );

private:

	void initScene();
	void updateScene( float deltaTime );
	void cameraMove( float deltaTime );
	void cameraMoveOnSphere( float deltaTime );
	
	bool isCameraRot;
	float cameraMoveForward;
	float cameraMoveRight;
	float cameraYaw;
	float cameraPitch;

	float cameraX;
	float cameraY;
	float cameraRadius;
	int lastMouseX;
	int lastMouseY;

	bool isDrawBg;
	bool isShowBoxTexture;

	GameTimer gameTimer;
	int windowWidth;
	int windowHeight;

private:

	class Camera *camera;
	class ScreenBuffer *screenBuffer;
	class PPMImage *texture;
	class DirectionalLight *mainLight;
	class ShapeRender *render;

	class Triangle *triangle;
	class Box *box;
};

void globalDraw();
void globalMainLoop();
void globalMouseEvent( int button , int state , int x , int y );
void globalMouseMove( int x , int y ); 
void globalKeyboardEvent( unsigned char key , int x , int y );
void globalKeyboardUpEvent( unsigned char key , int x , int y );