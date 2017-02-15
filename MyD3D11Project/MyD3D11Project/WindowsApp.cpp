#include "WindowsApp.h"

WindowsApp *globalInstance;
LRESULT CALLBACK WndProc( HWND hWnd , UINT msg , WPARAM wParam , LPARAM lParam )
{
	if ( globalInstance == nullptr )return 0;
	return globalInstance->MsgProc( hWnd , msg , wParam , lParam );
}


WindowsApp::WindowsApp( HINSTANCE hinstance , int show )
	:screenWidth( 1280 ) , screenHeight( 800 ) ,
	instanceHandle( hinstance ) , showCmd( show )
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
	window.hbrBackground = (HBRUSH) GetStockObject( WHITE_BRUSH );
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

LRESULT WindowsApp::MsgProc( HWND hWnd , UINT msg , WPARAM wParam , LPARAM lParam )
{
	switch ( msg )
	{
	case WM_LBUTTONDOWN:
		//MessageBox( 0 , L"HelloWorld" , L"Hello" , MB_OK );
		return 0;

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
	while ( msg.message != WM_QUIT )
	{
		// If there are Window messages then process them.
		if ( PeekMessage( &msg , 0 , 0 , 0 , PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else// Otherwise , do animation / game stuff.
		{

		}
	}
	return (int) msg.wParam;
}
