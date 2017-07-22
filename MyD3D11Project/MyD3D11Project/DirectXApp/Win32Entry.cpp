#include "SimpleScene.h"
#include "WaveScene.h"
#include "TessellationHillScene.h"
#include <crtdbg.h>
#include "../Utilities/Utilities.h"

#define SCENE 1

int WINAPI WinMain( HINSTANCE hInstance , HINSTANCE hPrevInstance , LPSTR lpCmdLine , int nShowCmd )
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

#if SCENE == 0

	SimpleScene testApp( hInstance , nShowCmd );
	if ( !testApp.InitDirectApp() ) return 0;
	return testApp.MsgLoop();

#elif SCENE == 1

	WaveScene testApp( hInstance , nShowCmd );
	if ( !testApp.InitDirectApp() ) return 0;
	return testApp.MsgLoop();

#elif SCENE == 2

	TessellationHillScene testApp( hInstance , nShowCmd );
	if ( !testApp.InitDirectApp() ) return 0;
	return testApp.MsgLoop();

#endif
}