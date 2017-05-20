#include "TessellationHillScene.h"
#include "../Utilities/CommonHeader.h"
#include "../BasicShape/BasicQuad.h"
#include <fstream>
using namespace DirectX;


TessellationHillScene::TessellationHillScene( HINSTANCE hinstance , int show )
	:DirectXApp( hinstance , show ) , moveSpeed( 0.08f ) , rotSpeed( 0.1f ) ,
	isWDown( false ) , isADown( false ) , isSDown( false ) , isDDown( false )
{
	lastMousePos.x = 0;
	lastMousePos.y = 0;

	terrain = new BasicQuad();
	terrain->Position.y = -1;
}


TessellationHillScene::~TessellationHillScene()
{
	ReleaseCOM( waveVB );

	delete terrain;
}

bool TessellationHillScene::InitDirectApp()
{
	if ( !DirectXApp::InitDirectApp() ) return false;
	
	createObjects();
	
	camera.buildProjectMatrix( screenWidth , screenHeight );
	return true;
}

void TessellationHillScene::UpdateScene( float deltaTime )
{
	float forwordSpeed = 0.0f;
	float rightSpeed = 0.0f;

	if ( isWDown )//w
	{
		forwordSpeed = 0.1f * moveSpeed;
	}
	else if ( isSDown )//s
	{
		forwordSpeed = -0.1f * moveSpeed;
	}
	
	if ( isADown )//a
	{
		rightSpeed = -0.1f * moveSpeed;
	}
	else if ( isDDown )//d
	{
		rightSpeed = 0.1f * moveSpeed;
	}

	camera.UpdatePosition2( forwordSpeed , rightSpeed );
}

void TessellationHillScene::DrawScene()
{
	immediateContext->ClearRenderTargetView( backBufferView , reinterpret_cast<const float *>( &XMVectorSet( 0.2f , 0.2f , 0.2f , 1.0f ) ) );
	immediateContext->ClearDepthStencilView( depthBufferView , D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL , 1.0f , 0 );

	camera.buildViewMatrix();
	terrain->UpdateObjectEffect( &camera );

	//terrain
	UINT stride = sizeof( TessVertex );
	UINT offset = 0;
	immediateContext->IASetVertexBuffers( 0 , 1 , &waveVB , &stride , &offset );
	terrain->RenderObject( immediateContext );

	HR( swapChain->Present( 0 , 0 ) );
}

void TessellationHillScene::OnResize()
{
	DirectXApp::OnResize();

	camera.buildProjectMatrix( screenWidth , screenHeight );
}

void TessellationHillScene::OnMouseDown( WPARAM btnState , int x , int y )
{
	SetCapture( ghMainWnd );
}

void TessellationHillScene::OnMouseMove( WPARAM btnState , int x , int y )
{
	if ( ( btnState & MK_RBUTTON ) != 0 )
	{
		float deltaX = XMConvertToRadians( ( x - lastMousePos.x )*rotSpeed );
		float deltaY = XMConvertToRadians( ( y - lastMousePos.y )*rotSpeed );
		
		camera.UpdateRotation( deltaX , deltaY );
	}
	
	lastMousePos.x = x;
	lastMousePos.y = y;
}

void TessellationHillScene::OnMouseUp( WPARAM btnState , int x , int y )
{
	ReleaseCapture();
}

void TessellationHillScene::OnMouseWheel( int zDelta )
{
	camera.UpdatePosition2( zDelta * moveSpeed , 0.0f );
}

void TessellationHillScene::OnKeyDown( WPARAM keyCode )
{
	if ( keyCode == 0x57 )//w
	{
		isWDown = true;
	}
	else if ( keyCode == 0x53 )//s
	{
		isSDown = true;
	}
	else if ( keyCode == 0x41 )//a
	{
		isADown = true;
	}
	else if ( keyCode == 0x44 )//d
	{
		isDDown = true;
	}
}

void TessellationHillScene::OnKeyUp( WPARAM keyCode )
{
	if ( keyCode == 0x57 )//w
	{
		isWDown = false;
	}
	else if ( keyCode == 0x53 )//s
	{
		isSDown = false;
	}
	else if ( keyCode == 0x41 )//a
	{
		isADown = false;
	}
	else if ( keyCode == 0x44 )//d
	{
		isDDown = false;
	}
}

template<typename T>
void TessellationHillScene::createVertexBuffer( const T *vertices , UINT vertexNum , ID3D11Buffer *&vertexBuffer )
{
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof( T ) * vertexNum;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = vertices;

	HR( device->CreateBuffer( &bufferDesc , &initData , &vertexBuffer ) );
}

void TessellationHillScene::createObjects()
{
	terrain->InitShape( device );
	
	const std::vector<TessVertex> &waveVlist = terrain->getVertices();

	createVertexBuffer( &waveVlist[0] , waveVlist.size() , waveVB );
}
