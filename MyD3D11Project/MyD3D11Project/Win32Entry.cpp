#include "DirectXApp.h"

int WINAPI WinMain( HINSTANCE hInstance , HINSTANCE hPrevInstance , LPSTR lpCmdLine , int nShowCmd )
{
	DirectXApp testApp( hInstance , nShowCmd );

	if ( !testApp.initDirectApp() ) return 0;

	return testApp.MsgLoop();
}