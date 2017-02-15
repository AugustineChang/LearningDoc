#pragma once
#include <windows.h>

class WindowsApp
{
public:
	WindowsApp( HINSTANCE hinstance , int show );
	~WindowsApp();
	
	int MsgLoop();
	LRESULT MsgProc( HWND hWnd , UINT msg , WPARAM wParam , LPARAM lParam );

protected:
	
	bool InitWinApp();

	HINSTANCE  instanceHandle;
	int showCmd;

	HWND ghMainWnd = 0;
	int screenWidth;
	int screenHeight;
};

