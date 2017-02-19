#include "MyFirstDX11App.h"
#include "CommonHeader.h"


MyFirstDX11App::MyFirstDX11App( HINSTANCE hinstance , int show )
	:DirectXApp( hinstance , show )
{
}


MyFirstDX11App::~MyFirstDX11App()
{
}

void MyFirstDX11App::UpdateScene( float deltaTime )
{

}

void MyFirstDX11App::DrawScene()
{
	assert( immediateContext );
	assert( swapChain );

	immediateContext->ClearRenderTargetView( backBufferView , reinterpret_cast<const float *>( &Colors::Blue ) );
	immediateContext->ClearDepthStencilView( depthBufferView , D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL , 1.0f , 0 );

	HR( swapChain->Present( 0 , 0 ) );
}
