#ifndef WINDOWS_APP_H
#define WINDOWS_APP_H

#include <windows.h>
#include "GameTimer.h"

class WindowsApp
{
public:
	WindowsApp( HINSTANCE hinstance , int show );
	~WindowsApp();
	
	int MsgLoop();
	LRESULT MsgProc( HWND hWnd , UINT msg , WPARAM wParam , LPARAM lParam );

	virtual void OnResize() = 0;
	virtual void OnMouseDown( WPARAM btnState , int x , int y ) = 0;
	virtual void OnMouseMove( WPARAM btnState , int x , int y ) = 0;
	virtual void OnMouseUp( WPARAM btnState , int x , int y ) = 0;
	virtual void OnMouseWheel( int zDelta ) = 0;

	virtual void UpdateScene( float deltaTime ) = 0;
	virtual void DrawScene() = 0;

protected:
	
	bool InitWinApp();
	void CalcFrameStat();

	GameTimer gameTimer;

	HINSTANCE  instanceHandle;
	int showCmd;

	HWND ghMainWnd = 0;
	int screenWidth;
	int screenHeight;
	bool isDraging;

	int fpsCount;
	float fpsBaseTime;
};
#endif // !WINDOWS_APP_H
