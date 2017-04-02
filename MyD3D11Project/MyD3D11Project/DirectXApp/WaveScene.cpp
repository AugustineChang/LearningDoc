#include "WaveScene.h"
#include "../Utilities/CommonHeader.h"
#include "../BasicShape/WaveTerrain.h"
#include <fstream>
using namespace DirectX;


WaveScene::WaveScene( HINSTANCE hinstance , int show )
	:DirectXApp( hinstance , show ) , moveSpeed( 0.1f ) , 
	radius( 5.0f ) , zoomSpeed( 0.005f )
{
	lastMousePos.x = 0;
	lastMousePos.y = 0;

	wave = new WaveTerrain();
	terrain = new SimpleTerrain();
}


WaveScene::~WaveScene()
{
	ReleaseCOM( inputLayout );
	ReleaseCOM( rasterState );

	delete wave;
	delete terrain;
}

bool WaveScene::InitDirectApp()
{
	if ( !DirectXApp::InitDirectApp() ) return false;

	effect.createEffectAtBuildtime( device );
	createInputLayout();
	createObjects();
	createRenderState();

	camera.Position.z = -radius;
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

	immediateContext->IASetInputLayout( inputLayout );
	immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	immediateContext->RSSetState( rasterState );

	wave->buildWorldMatrix();
	terrain->buildWorldMatrix();
	camera.buildViewMatrix();

	effect.UpdateSceneEffect( &camera , &dirLight , nullptr , nullptr );

	//terrain
	UINT stride = sizeof( CustomVertex );
	UINT offset = 0;
	immediateContext->IASetVertexBuffers( 0 , 1 , &otherVB , &stride , &offset );
	immediateContext->IASetIndexBuffer( otherIB , DXGI_FORMAT_R32_UINT , 0 );
	renderObject( *terrain , terrain->indexSize , terrain->indexStart , terrain->indexBase );

	//wave
	stride = sizeof( CustomVertex );
	offset = 0;
	immediateContext->IASetVertexBuffers( 0 , 1 , &waveVB , &stride , &offset );
	immediateContext->IASetIndexBuffer( waveIB , DXGI_FORMAT_R32_UINT , 0 );
	renderObject( *wave , wave->indexSize , wave->indexStart , wave->indexBase );

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
		float deltaX = XMConvertToRadians( ( x - lastMousePos.x )*moveSpeed );
		float deltaY = XMConvertToRadians( ( y - lastMousePos.y )*moveSpeed );
		
		camera.Rotation.y += deltaX;
		camera.Rotation.x = SimpleMath::Clamp<float>( camera.Rotation.x + deltaY , -SimpleMath::PI / 2 + 0.01f , SimpleMath::PI / 2 - 0.01f );

		camera.Position.x = radius * cosf( camera.Rotation.x ) * cosf( -camera.Rotation.y - SimpleMath::PI / 2 );
		camera.Position.z = radius * cosf( camera.Rotation.x ) * sinf( -camera.Rotation.y - SimpleMath::PI / 2 );
		camera.Position.y = radius * sinf( camera.Rotation.x );

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
	radius -= zDelta * zoomSpeed;

	camera.Position.x = radius * cosf( camera.Rotation.x ) * cosf( -camera.Rotation.y - SimpleMath::PI / 2 );
	camera.Position.z = radius * cosf( camera.Rotation.x ) * sinf( -camera.Rotation.y - SimpleMath::PI / 2 );
	camera.Position.y = radius * sinf( camera.Rotation.x );
}

void WaveScene::createInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC descList[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};

	/*D3D11_INPUT_ELEMENT_DESC descList2[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,24,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32B32_FLOAT,0,36,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",1,DXGI_FORMAT_R32G32B32_FLOAT,0,48,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,60,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};*/

	D3DX11_PASS_DESC passDesc;
	effect.getEffectTech()->GetPassByIndex( 0 )->GetDesc( &passDesc );

	HR( device->CreateInputLayout( descList , 3 , passDesc.pIAInputSignature , passDesc.IAInputSignatureSize , &inputLayout ) );
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

void WaveScene::createRenderState()
{
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory( &rsDesc , sizeof( D3D11_RASTERIZER_DESC ) );
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;
	
	device->CreateRasterizerState( &rsDesc , &rasterState );
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

void WaveScene::renderObject( const BasicShape &basicObj , UINT indexSize , UINT indexStart , UINT indexBase )
{
	XMMATRIX &tempW = basicObj.getWorldMatrix();
	XMMATRIX &tempV = camera.getViewMatrix();
	XMMATRIX &tempP = camera.getProjectMatrix();
	XMMATRIX tempWVP = tempW * tempV * tempP;

	XMMATRIX inverseW = XMMatrixInverse( &XMMatrixDeterminant( tempW ) , tempW );
	XMMATRIX inverseTransposeW = XMMatrixTranspose( inverseW );
	XMMATRIX identityMat = XMMatrixIdentity();

	float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	immediateContext->OMSetBlendState( basicObj.getBlendState() , blendFactors , 0xffffffff );
	effect.UpdateObjectEffect( tempWVP , tempW , inverseTransposeW , identityMat , &basicObj );

	D3DX11_TECHNIQUE_DESC techDesc;
	effect.getEffectTech()->GetDesc( &techDesc );
	for ( UINT i = 0; i < techDesc.Passes; ++i )
	{
		effect.getEffectTech()->GetPassByIndex( i )->Apply( 0 , immediateContext );

		immediateContext->DrawIndexed( indexSize , indexStart , indexBase );
	}
}
