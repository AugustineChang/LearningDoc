#include "WindowsApp.h"
#include <windowsX.h>
#include <sstream>

WindowsApp *globalInstance;
LRESULT CALLBACK WndProc( HWND hWnd , UINT msg , WPARAM wParam , LPARAM lParam )
{
	if ( globalInstance == nullptr )return 0;
	return globalInstance->MsgProc( hWnd , msg , wParam , lParam );
}


WindowsApp::WindowsApp( HINSTANCE hinstance , int show )
	:screenWidth( 1280 ) , screenHeight( 800 ) , isDraging( false ) ,
	instanceHandle( hinstance ) , showCmd( show ) ,
	fpsCount( 0 ) , fpsBaseTime( 0.0f )
{
	globalInstance = this;
}


WindowsApp::~WindowsApp()
{
}

bool WindowsApp::InitWinApp()
{
	WNDCLASS window;

	window.style = CS_HREDRAW | CS_VREDRAW;
	window.lpfnWndProc = WndProc;
	window.cbClsExtra = 0;
	window.cbWndExtra = 0;
	window.hInstance = instanceHandle;
	window.hIcon = LoadIcon( 0 , IDI_APPLICATION );
	window.hCursor = LoadCursor( 0 , IDC_ARROW );
	window.hbrBackground = (HBRUSH) GetStockObject( NULL_BRUSH );
	window.lpszMenuName = 0;
	window.lpszClassName = L"MyBasicWindow";

	if ( !RegisterClass( &window ) )
	{
		MessageBox( 0 , L"Register Window Failed!!" , 0 , 0 );
		return false;
	}

	ghMainWnd = CreateWindow
	(
		L"MyBasicWindow" ,
		L"MyFirstWindow" ,
		WS_OVERLAPPEDWINDOW ,
		CW_USEDEFAULT ,
		CW_USEDEFAULT ,
		CW_USEDEFAULT ,
		CW_USEDEFAULT ,
		0 ,
		0 ,
		instanceHandle ,
		0
	);

	if ( ghMainWnd == 0 )
	{
		MessageBox( 0 , L"Create Window Failed!!" , 0 , 0 );
		return false;
	}

	ShowWindow( ghMainWnd , showCmd );
	UpdateWindow( ghMainWnd );
	return true;
}

void WindowsApp::CalcFrameStat()
{
	/*
	//Debug Code

	std::wostringstream ss;
	ss.setf( std::ios::fixed , std::ios::floatfield );
	ss.precision( 2 );
	ss << L"DeltaTime:" << gameTimer.DeltaTime() * 1000 << L"ms";
	ss << L" TotalTime:" << gameTimer.TotalTime();
	SetWindowText( ghMainWnd , ss.str().c_str() );
	*/

	fpsCount++;

	float passedTime = gameTimer.TotalTime() - fpsBaseTime;
	if ( passedTime >= 1.0f )
	{
		float fps = fpsCount / passedTime;
		float mspf = passedTime / fpsCount * 1000.0f;

		std::wostringstream ss;
		ss.setf( std::ios::fixed , std::ios::floatfield );
		ss.precision( 2 );
		if ( fps > 1000.0f )
		{
			ss << L"FPS:" << ( fps / 1000.0f ) << L"K ";
			ss << L"FrameTime:" << ( mspf * 1000.0f ) << L"us ";
		}
		else
		{
			ss << L"FPS:" << fps << L" ";
			ss << L"FrameTime:" << mspf << L"ms ";
		}
		SetWindowText( ghMainWnd , ss.str().c_str() );

		fpsCount = 0;
		fpsBaseTime = gameTimer.TotalTime();
	}
}

LRESULT WindowsApp::MsgProc( HWND hWnd , UINT msg , WPARAM wParam , LPARAM lParam )
{
	switch ( msg )
	{
	case WM_ACTIVATE:
		if ( LOWORD( wParam ) == WA_INACTIVE )
		{
			gameTimer.Pause();
		}
		else
		{
			gameTimer.UnPause();
		}
		return 0;

	case WM_SIZE:
		screenWidth = LOWORD( lParam );
		screenHeight = HIWORD( lParam );

		if ( wParam == SIZE_MINIMIZED )
		{
			gameTimer.Pause();
		}
		else if ( gameTimer.isPaused() )
		{
			gameTimer.UnPause();

			if ( !isDraging ) OnResize();
		}
		return 0;

	case WM_ENTERSIZEMOVE:
		gameTimer.Pause();
		isDraging = true;
		return 0;
	case WM_EXITSIZEMOVE:
		gameTimer.UnPause();
		isDraging = false;
		OnResize();
		return 0;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		OnMouseDown( wParam , GET_X_LPARAM( lParam ) , GET_Y_LPARAM( lParam ) );
		return 0;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		OnMouseUp( wParam , GET_X_LPARAM( lParam ) , GET_Y_LPARAM( lParam ) );
		return 0;

	case WM_MOUSEMOVE:
		OnMouseMove( wParam , GET_X_LPARAM( lParam ) , GET_Y_LPARAM( lParam ) );
		return 0;

	case WM_GETMINMAXINFO:
		( (MINMAXINFO*) lParam )->ptMinTrackSize.x = 200;
		( (MINMAXINFO*) lParam )->ptMinTrackSize.y = 200;
		return 0;

	case WM_MENUCHAR:
		return MAKELRESULT( 0 , MNC_CLOSE );

	case WM_KEYDOWN:
		if ( wParam == VK_ESCAPE )
		{
			DestroyWindow( ghMainWnd );
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage( 0 );
		return 0;
	}

	return DefWindowProc( hWnd , msg , wParam , lParam );
}

int WindowsApp::MsgLoop()
{
	MSG msg = { 0 };
	gameTimer.Reset();

	while ( msg.message != WM_QUIT )
	{
		if ( PeekMessage( &msg , 0 , 0 , 0 , PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			gameTimer.Tick();

			if ( !gameTimer.isPaused() )
			{
				CalcFrameStat();
				UpdateScene( gameTimer.DeltaTime() );
				DrawScene();
			}
			else Sleep( 100 );
		}
	}
	return (int) msg.wParam;
}
