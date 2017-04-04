#include "WaveScene.h"
#include "../Utilities/CommonHeader.h"
#include "../BasicShape/WaveTerrain.h"
#include "../BasicShape/BasicCube.h"
#include <fstream>
using namespace DirectX;


WaveScene::WaveScene( HINSTANCE hinstance , int show )
	:DirectXApp( hinstance , show ) , moveSpeed( 0.08f ) , rotSpeed( 0.1f )
{
	lastMousePos.x = 0;
	lastMousePos.y = 0;

	wave = new WaveTerrain();
	terrain = new SimpleTerrain();
	box = new BasicCube();
	box->Position.y = 2.0f;
	box->Scale = XMFLOAT3( 3.0f , 3.0f , 3.0f );
}


WaveScene::~WaveScene()
{
	ReleaseCOM( waveVB );
	ReleaseCOM( waveIB );
	ReleaseCOM( otherVB );
	ReleaseCOM( otherIB );

	delete wave;
	delete terrain;
	delete box;
}

bool WaveScene::InitDirectApp()
{
	if ( !DirectXApp::InitDirectApp() ) return false;
	
	createObjects();
	
	camera.buildProjectMatrix( screenWidth , screenHeight );
	return true;
}

void WaveScene::UpdateScene( float deltaTime )
{
	wave->UpdateObject( deltaTime );
	UINT len = wave->getVertices().size();

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR( immediateContext->Map( waveVB , 0 , D3D11_MAP_WRITE_DISCARD , 0 , &mappedData ) );
	CustomVertex *vert = reinterpret_cast<CustomVertex *>( mappedData.pData );
	for ( UINT i = 0; i < len; ++i )
	{
		vert[i].Pos = wave->getVertices()[i].Pos;
		vert[i].Normal = wave->getVertices()[i].Normal;
		vert[i].TexCoord = wave->getVertices()[i].TexCoord;
	}
	immediateContext->Unmap( waveVB , 0 );

	//move texture
	moveOffset.x += 0.05f * deltaTime;
	moveOffset.y += 0.11f * deltaTime;
	moveMatrix = XMMatrixTranslation( moveOffset.x , moveOffset.y , 0.0f );
}

void WaveScene::DrawScene()
{
	immediateContext->ClearRenderTargetView( backBufferView , reinterpret_cast<const float *>( &XMVectorSet( 0.2f , 0.2f , 0.2f , 1.0f ) ) );
	immediateContext->ClearDepthStencilView( depthBufferView , D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL , 1.0f , 0 );

	immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	camera.buildViewMatrix();
	terrain->UpdateObjectEffect( &camera , &dirLight );
	wave->UpdateObjectEffect( &camera , &dirLight );
	box->UpdateObjectEffect( &camera , &dirLight );

	//terrain
	UINT stride = sizeof( CustomVertex );
	UINT offset = 0;
	immediateContext->IASetVertexBuffers( 0 , 1 , &otherVB , &stride , &offset );
	immediateContext->IASetIndexBuffer( otherIB , DXGI_FORMAT_R32_UINT , 0 );
	terrain->RenderObject( immediateContext );
	box->RenderObject( immediateContext );

	//wave
	immediateContext->IASetVertexBuffers( 0 , 1 , &waveVB , &stride , &offset );
	immediateContext->IASetIndexBuffer( waveIB , DXGI_FORMAT_R32_UINT , 0 );
	wave->RenderObject( immediateContext );

	HR( swapChain->Present( 0 , 0 ) );
}

void WaveScene::OnResize()
{
	DirectXApp::OnResize();

	camera.buildProjectMatrix( screenWidth , screenHeight );
}

void WaveScene::OnMouseDown( WPARAM btnState , int x , int y )
{
	lastMousePos.x = x;
	lastMousePos.y = y;

	SetCapture( ghMainWnd );
}

void WaveScene::OnMouseMove( WPARAM btnState , int x , int y )
{
	if ( ( btnState & MK_RBUTTON ) != 0 )
	{
		float deltaX = XMConvertToRadians( ( x - lastMousePos.x )*rotSpeed );
		float deltaY = XMConvertToRadians( ( y - lastMousePos.y )*rotSpeed );
		
		camera.UpdateRotation( deltaX , deltaY );
		
		lastMousePos.x = x;
		lastMousePos.y = y;
	}
}

void WaveScene::OnMouseUp( WPARAM btnState , int x , int y )
{
	ReleaseCapture();
}

void WaveScene::OnMouseWheel( int zDelta )
{
	camera.UpdatePosition2( zDelta * moveSpeed );
}

template<typename T>
void WaveScene::createVertexBuffer( const T *vertices , UINT vertexNum , ID3D11Buffer *&vertexBuffer )
{
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof( T ) * vertexNum;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = vertices;

	HR( device->CreateBuffer( &bufferDesc , &initData , &vertexBuffer ) );
}

template<typename T>
void WaveScene::createVertexBuffer2( const T *vertices , UINT vertexNum , ID3D11Buffer *&vertexBuffer )
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

void WaveScene::createIndexBuffer( const UINT *indices , UINT indexNum , ID3D11Buffer *&indexBuffer )
{
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof( UINT ) * indexNum;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = indices;

	HR( device->CreateBuffer( &bufferDesc , &initData , &indexBuffer ) );
}

void WaveScene::createObjects()
{
	std::vector<CustomVertex> waveVlist;
	std::vector<UINT> waveIlist;
	std::vector<CustomVertex> gvlist;
	std::vector<UINT> gilist;

	wave->InitShape( device );
	addToGlobalBuffer( waveVlist , waveIlist , *wave );

	terrain->InitShape( device );
	addToGlobalBuffer( gvlist , gilist , *terrain );
	box->InitShape( device );
	addToGlobalBuffer( gvlist , gilist , *box );

	createVertexBuffer( &waveVlist[0] , waveVlist.size() ,waveVB );
	createIndexBuffer( &waveIlist[0] , waveIlist.size() , waveIB );
	createVertexBuffer2( &gvlist[0] , gvlist.size() , otherVB );
	createIndexBuffer( &gilist[0] , gilist.size() , otherIB );
}

void WaveScene::addToGlobalBuffer( std::vector<CustomVertex> &gVBuffer , std::vector<UINT> &gIBuffer , BasicShape &shape )
{
	std::vector<CustomVertex> vlist = shape.getVertices();
	std::vector<UINT> ilist = shape.getIndices();

	shape.indexSize = ilist.size();
	shape.indexStart = gIBuffer.size();
	shape.indexBase = gVBuffer.size();

	gVBuffer.insert( gVBuffer.end() , vlist.begin() , vlist.end() );
	gIBuffer.insert( gIBuffer.end() , ilist.begin() , ilist.end() );
}
