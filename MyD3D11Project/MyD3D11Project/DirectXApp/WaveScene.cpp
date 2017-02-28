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
}


WaveScene::~WaveScene()
{
	ReleaseCOM( inputLayout );
	ReleaseCOM( effect );
	ReleaseCOM( rasterState );

	delete wave;
}

bool WaveScene::InitDirectApp()
{
	if ( !DirectXApp::InitDirectApp() ) return false;

	createEffectAtBuildtime();
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
	HR( immediateContext->Map( waveVB[0] , 0 , D3D11_MAP_WRITE_DISCARD , 0 , &mappedData ) );
	XMFLOAT3 *vert = reinterpret_cast<XMFLOAT3 *>( mappedData.pData );
	for ( UINT i = 0; i < len; ++i )
	{
		vert[i] = wave->getVertices()[i].Pos;
	}
	immediateContext->Unmap( waveVB[0] , 0 );

	D3D11_MAPPED_SUBRESOURCE mappedData2;
	HR( immediateContext->Map( waveVB[1] , 0 , D3D11_MAP_WRITE_DISCARD , 0 , &mappedData2 ) );
	XMFLOAT4 *vert2 = reinterpret_cast<XMFLOAT4 *>( mappedData2.pData );
	for ( UINT i = 0; i < len; ++i )
	{
		float yPos = wave->getVertices()[i].Pos.y;
		float color = ( yPos + 1.0f ) / 6.0f;

		vert2[i] = XMFLOAT4( 0.2f , 1.0f - color , color , 1.0f );
	}
	immediateContext->Unmap( waveVB[1] , 0 );
}

void WaveScene::DrawScene()
{
	immediateContext->ClearRenderTargetView( backBufferView , reinterpret_cast<const float *>( &XMVectorSet( 0.2f , 0.2f , 0.2f , 1.0f ) ) );
	immediateContext->ClearDepthStencilView( depthBufferView , D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL , 1.0f , 0 );

	immediateContext->IASetInputLayout( inputLayout );
	immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	UINT stride[2] = { sizeof( XMFLOAT3 ),sizeof( XMFLOAT4 ) };
	UINT offset[2] = { 0,0 };
	immediateContext->IASetVertexBuffers( 0 , 2 , &waveVB[0] , &stride[0] , &offset[0] );
	immediateContext->IASetIndexBuffer( waveIB , DXGI_FORMAT_R32_UINT , 0 );
	immediateContext->RSSetState( rasterState );
	
	wave->buildWorldMatrix();
	camera.buildViewMatrix();

	CXMMATRIX tempW = wave->getWorldMatrix();
	CXMMATRIX tempV = camera.getViewMatrix();
	CXMMATRIX tempP = camera.getProjectMatrix();
	XMMATRIX tempWVP = tempW * tempV * tempP;
	effectWVP->SetMatrix( reinterpret_cast<float*>( &tempWVP ) );

	D3DX11_TECHNIQUE_DESC techDesc;
	effectTech->GetDesc( &techDesc );
	UINT indexSize = wave->getIndices().size();
	for ( UINT i = 0; i < techDesc.Passes; ++i )
	{
		effectTech->GetPassByIndex( i )->Apply( 0 , immediateContext );
		immediateContext->DrawIndexed( indexSize , 0 , 0 );
	}

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
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,1,0,D3D11_INPUT_PER_VERTEX_DATA,0}
	};
	/*D3D11_INPUT_ELEMENT_DESC descList[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};*/


	/*D3D11_INPUT_ELEMENT_DESC descList2[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,24,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32B32_FLOAT,0,36,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",1,DXGI_FORMAT_R32G32B32_FLOAT,0,48,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,60,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};*/

	effectTech = effect->GetTechniqueByName( "SimpleTech" );
	D3DX11_PASS_DESC passDesc;
	effectTech->GetPassByIndex( 0 )->GetDesc( &passDesc );

	HR( device->CreateInputLayout( descList , 2 , passDesc.pIAInputSignature , passDesc.IAInputSignatureSize , &inputLayout ) );
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
	rsDesc.FillMode = D3D11_FILL_WIREFRAME;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;
	
	device->CreateRasterizerState( &rsDesc , &rasterState );
}

void WaveScene::createEffectAtRuntime()
{
	DWORD shaderFlag = 0;
#if defined(DEBUG) | defined(_DEBUG)
	shaderFlag |= D3D10_SHADER_DEBUG;
	shaderFlag |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	
	ID3D10Blob *compilationMsgs;
	HRESULT hr = D3DX11CompileEffectFromFile( L"SimpleShader.fx" , 0 , 0 , shaderFlag , 0 , device , &effect , &compilationMsgs );

	// compilationMsgs can store errors or warnings.
	if ( compilationMsgs != 0 )
	{
		MessageBoxA( 0 , (char*) compilationMsgs->GetBufferPointer() , 0 , 0 );
		ReleaseCOM( compilationMsgs );
	}

	if ( FAILED( hr ) )
	{
		DXTrace( __FILEW__ , (DWORD) __LINE__ , hr ,
			L"D3DX11CompileFromFile" , true );
	}

	effectWVP = effect->GetVariableByName( "gWVP" )->AsMatrix();
}

void WaveScene::createEffectAtBuildtime()
{
	std::ifstream fs( "FX/SimpleShader.fxo" , std::ios::binary );
	assert( fs );

	fs.seekg( 0 , std::ios_base::end );
	size_t size = (size_t) fs.tellg();
	fs.seekg( 0 , std::ios_base::beg );
	std::vector<char> compiledShader( size );
	fs.read( &compiledShader[0] , size );
	fs.close();

	HR( D3DX11CreateEffectFromMemory( &compiledShader[0] , size , 0 , device , &effect ) );
	effectWVP = effect->GetVariableByName( "gWVP" )->AsMatrix();
}

void WaveScene::createObjects()
{
	std::vector<CustomVertex> vlist = wave->getVertices();
	std::vector<UINT> ilist = wave->getIndices();
	

	std::vector<XMFLOAT3> vertex_Pos;
	std::vector<XMFLOAT4> vertex_Col;

	for ( const CustomVertex& vert : vlist )
	{
		vertex_Pos.push_back( vert.Pos );
		vertex_Col.push_back( vert.Color );
	}

	UINT vertSize = vlist.size();

	//createVertexBuffer<CustomVertex>( &vlist[0] , vertSize , waveVB );
	createVertexBuffer<XMFLOAT3>( &vertex_Pos[0] , vertSize , waveVB[0] );
	createVertexBuffer<XMFLOAT4>( &vertex_Col[0] , vertSize , waveVB[1] );
	createIndexBuffer( &ilist[0] , ilist.size() , waveIB );
}
