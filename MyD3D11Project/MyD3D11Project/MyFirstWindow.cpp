#include <windows.h>

HWND ghMainWnd = 0;

bool InitWindowsApp( HINSTANCE  instanceHandle , int show );
LRESULT CALLBACK WndProc( HWND hWnd , UINT msg , WPARAM wParam , LPARAM lParam );
int MsgLoop();

//int WINAPI WinMain( HINSTANCE hInstance , HINSTANCE hPreInstance , PSTR pCmdLine , int nShowCmd )
//{
//	if ( !InitWindowsApp( hInstance , nShowCmd ) )
//	{
//		return 0;
//	}
//
//	return MsgLoop();
//}

bool InitWindowsApp( HINSTANCE  instanceHandle , int show )
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

	ShowWindow( ghMainWnd , show );
	UpdateWindow( ghMainWnd );
	return true;
}


LRESULT CALLBACK
WndProc( HWND hWnd , UINT msg , WPARAM wParam , LPARAM lParam )
{
	switch ( msg )
	{
	case WM_LBUTTONDOWN:
		MessageBox( 0 , L"HelloWorld" , L"Hello" , MB_OK );
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

int MsgLoop()
{
	MSG msg = { 0 };

	BOOL bRet = 1;
	while ( ( bRet = GetMessage( &msg , 0 , 0 , 0 ) ) != 0 )
	{
		if ( bRet == -1 )
		{
			MessageBox( 0 , L"Get Message Failed!!!" , 0 , 0 );
			break;
		}
		else
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	return (int) msg.wParam;
}