#include "MyFirstDX11App.h"
#include <crtdbg.h>
#include "Utilities.h"

int WINAPI WinMain( HINSTANCE hInstance , HINSTANCE hPrevInstance , LPSTR lpCmdLine , int nShowCmd )
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	MyFirstDX11App testApp( hInstance , nShowCmd );

	if ( !testApp.InitDirectApp() ) return 0;

	//testApp.QueryGraphicAdapters();

	return testApp.MsgLoop();
}