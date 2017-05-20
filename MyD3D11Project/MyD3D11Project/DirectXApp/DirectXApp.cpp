#include "DirectXApp.h"
#include "../Utilities/CommonHeader.h"
#include <sstream>

DirectXApp::DirectXApp( HINSTANCE hinstance , int show )
	:WindowsApp( hinstance , show ) , enable4xMSAA( true )
{
}


DirectXApp::~DirectXApp()
{
	ReleaseCOM( backBufferView );
	ReleaseCOM( depthBufferView );
	ReleaseCOM( depthBuffer );
	ReleaseCOM( swapChain );

	if ( immediateContext )immediateContext->ClearState();

	ReleaseCOM( immediateContext );
	ReleaseCOM( device );
}

bool DirectXApp::InitDirectApp()
{
	if ( !InitWinApp() ) return false;

	if ( !createDirectXDevice() ) return false;
	checkMSAAQuality();
	createSwapChain();
	createBackBufferView();
	createDepthBufferView();
	initImmediateContext();

	return true;
}

void DirectXApp::OnResize()
{
	if ( device == nullptr ) return;
	if ( immediateContext == nullptr ) return;
	if ( swapChain == nullptr ) return;

	ReleaseCOM( backBufferView );
	ReleaseCOM( depthBufferView );
	ReleaseCOM( depthBuffer );

	swapChain->ResizeBuffers( 1 , screenWidth , screenHeight , DXGI_FORMAT_R8G8B8A8_UNORM , 0 );
	createBackBufferView();
	createDepthBufferView();
	initImmediateContext();
}

void DirectXApp::OnMouseDown( WPARAM btnState , int x , int y ){}
void DirectXApp::OnMouseMove( WPARAM btnState , int x , int y ){}
void DirectXApp::OnMouseUp( WPARAM btnState , int x , int y ){}

void DirectXApp::OnKeyDown( WPARAM keyCode ) {}
void DirectXApp::OnKeyUp( WPARAM keyCode ) {}

void DirectXApp::UpdateScene( float deltaTime ){}
void DirectXApp::DrawScene(){}

bool DirectXApp::createDirectXDevice()
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

void DirectXApp::checkMSAAQuality()
{
	device->CheckMultisampleQualityLevels( DXGI_FORMAT_R8G8B8A8_UNORM , 4 , &m4xMsaaQuality );
	assert( m4xMsaaQuality > 0 );
}

void DirectXApp::createSwapChain()
{
	//describe swap chain
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
		scd.SampleDesc.Quality = m4xMsaaQuality - 1;
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

	//create swap chain
	IDXGIDevice *dxgiDevice;
	device->QueryInterface( __uuidof( IDXGIDevice ) , (void**) &dxgiDevice );
	IDXGIAdapter *dxgiAdapter;
	dxgiDevice->GetParent( __uuidof( IDXGIAdapter ) , (void**) &dxgiAdapter );
	IDXGIFactory *dxgiFactory;
	dxgiAdapter->GetParent( __uuidof( IDXGIFactory ) , (void**) &dxgiFactory );

	HR( dxgiFactory->CreateSwapChain( device , &scd , &swapChain ) );
	HR( dxgiFactory->MakeWindowAssociation( ghMainWnd , DXGI_MWA_NO_WINDOW_CHANGES ) );

	ReleaseCOM( dxgiDevice );
	ReleaseCOM( dxgiAdapter );
	ReleaseCOM( dxgiFactory );
}

void DirectXApp::createBackBufferView()
{
	ID3D11Texture2D *backBuffer;
	swapChain->GetBuffer( 0 , __uuidof( ID3D11Texture2D ) , reinterpret_cast<void**>( &backBuffer ) );
	HR( device->CreateRenderTargetView( backBuffer , nullptr , &backBufferView ) );
	ReleaseCOM( backBuffer );
}

void DirectXApp::createDepthBufferView()
{
	//create Depth Buffer
	D3D11_TEXTURE2D_DESC depthTexDesc;
	depthTexDesc.Width = screenWidth;
	depthTexDesc.Height = screenHeight;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	if ( enable4xMSAA )
	{
		depthTexDesc.SampleDesc.Count = 4;
		depthTexDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	else
	{
		depthTexDesc.SampleDesc.Count = 1;
		depthTexDesc.SampleDesc.Quality = 0;
	}

	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;
	
	HR( device->CreateTexture2D( &depthTexDesc , nullptr , &depthBuffer ) );
	HR( device->CreateDepthStencilView( depthBuffer , nullptr , &depthBufferView ) );
}

void DirectXApp::initImmediateContext()
{
	immediateContext->OMSetRenderTargets( 1 , &backBufferView , depthBufferView );

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float) screenWidth;
	viewport.Height = (float) screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	immediateContext->RSSetViewports( 1 , &viewport );
}

void DirectXApp::QueryGraphicAdapters()
{
	IDXGIFactory *dxgiFactory;
	CreateDXGIFactory( __uuidof( IDXGIFactory ) , (void**) &dxgiFactory );

	std::wostringstream ss;
	UINT adapterIndex = 0;
	IDXGIAdapter *adapter;
	while ( dxgiFactory->EnumAdapters( adapterIndex , &adapter ) != DXGI_ERROR_NOT_FOUND )
	{
		LARGE_INTEGER* umdVersion = nullptr;
		HR( adapter->CheckInterfaceSupport( __uuidof( ID3D10Device ) , umdVersion ) );

		ss << L"Adapter " << adapterIndex << L"\n";
		ss << L"\tUMDVersion " << umdVersion << L"\n";

		UINT outputIndex = 0;
		IDXGIOutput *output;
		while ( adapter->EnumOutputs( outputIndex , &output ) != DXGI_ERROR_NOT_FOUND )
		{
			ss << L"\tOutput " << outputIndex << L"\n";

			UINT displayModeNum = 0;
			DXGI_FORMAT displayFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			UINT displayFlag = DXGI_ENUM_MODES_INTERLACED;

			HR( output->GetDisplayModeList( displayFormat , 0 , &displayModeNum , nullptr ) );
			DXGI_MODE_DESC *modeDesc = new DXGI_MODE_DESC[displayModeNum];
			HR( output->GetDisplayModeList( displayFormat , 0 , &displayModeNum , modeDesc ) );

			for ( UINT index = 0; index < displayModeNum; ++index )
			{
				ss << L"\t\tDisplayMode: W=" << modeDesc[index].Width << L" H=" << modeDesc[index].Height
					<< "RefreshRate=" << modeDesc[index].RefreshRate.Numerator << L"/" << modeDesc[index].RefreshRate.Denominator << L"\n";
			}

			delete[] modeDesc;
			++outputIndex;
		}
		
		if ( outputIndex == 0 ) ss << L"\tNo Output" << "\n";

		++adapterIndex;
	}
	
	OutputDebugString( ss.str().c_str() );
}
