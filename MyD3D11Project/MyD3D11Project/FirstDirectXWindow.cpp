#include <windows.h>
#include <assert.h>
#include <d3d11.h>
#include "MyConstant.h"

bool createDevice( ID3D11Device *&device , ID3D11DeviceContext *&immediateContext );
HWND ghMainWnd = 0;

int WINAPI WinMain( HINSTANCE hInstance , HINSTANCE hPrevInstance , LPSTR lpCmdLine , int nShowCmd )
{
	ID3D11Device *device;
	ID3D11DeviceContext *immediateContext;

	if ( !createDevice( device , immediateContext ) )return 0;

	UINT m4xMsaaQuality;
	device->CheckMultisampleQualityLevels( DXGI_FORMAT_R8G8B8A8_UNORM , 4 , &m4xMsaaQuality );
	assert( m4xMsaaQuality > 0 );

	return 0;
}


bool createDevice( ID3D11Device *&device , ID3D11DeviceContext *&immediateContext )
{
	UINT createDeviceFlag = 0;

#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featrueLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3
	};
	
	D3D_FEATURE_LEVEL supportFeature;

	HRESULT hr = D3D11CreateDevice
	(
		nullptr ,
		D3D_DRIVER_TYPE_HARDWARE ,
		nullptr ,
		createDeviceFlag ,
		featrueLevels ,
		sizeof( featrueLevels ) / sizeof( D3D_FEATURE_LEVEL ) ,
		D3D11_SDK_VERSION ,
		&device ,
		&supportFeature ,
		&immediateContext
	);

	if ( FAILED( hr ) )
	{
		MessageBox( 0 , L"Create D3D11Device Failed!!" , 0 , 0 );
		return false;
	}

	if ( supportFeature < D3D_FEATURE_LEVEL_11_0 )
	{
		MessageBox( 0 , L"Not Support Direc3D Feature Level 11 !!" , 0 , 0 );
		return false;
	}

	return true;
}

void createSwapChain( bool enable4xMSAA , UINT msaaQuality )
{
	DXGI_SWAP_CHAIN_DESC scd;

	scd.BufferDesc.Width = screenWidth;
	scd.BufferDesc.Height = screenHeight;
	scd.BufferDesc.RefreshRate.Denominator = 60;
	scd.BufferDesc.RefreshRate.Numerator = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	
	if ( enable4xMSAA )
	{
		scd.SampleDesc.Count = 4;
		scd.SampleDesc.Quality = msaaQuality - 1;
	}
	else
	{
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
	}

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 1;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.OutputWindow = ghMainWnd;
	scd.Windowed = true;
	scd.Flags = 0;
}