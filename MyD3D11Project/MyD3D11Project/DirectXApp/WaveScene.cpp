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

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR( immediateContext->Map( waveVB , 0 , D3D11_MAP_WRITE_DISCARD , 0 , &mappedData ) );
	CustomVertex *vert = reinterpret_cast<CustomVertex *>( mappedData.pData );
	UINT len = wave->getVertices().size();
	for ( UINT i = 0; i < len; ++i )
	{
		vert[i].Pos = wave->getVertices()[i].Pos;

		float yPos = vert[i].Pos.y;
		float color = ( yPos + 1.0f ) / 2.0f;

		vert[i].Pos = wave->getVertices()[i].Pos;
		vert[i].Color = XMFLOAT4( 0.2f , 1.0f - color , color , 1.0f );
	}
	immediateContext->Unmap( waveVB , 0 );
}

void WaveScene::DrawScene()
{
	immediateContext->ClearRenderTargetView( backBufferView , reinterpret_cast<const float *>( &XMVectorSet( 0.2f , 0.2f , 0.2f , 1.0f ) ) );
	immediateContext->ClearDepthStencilView( depthBufferView , D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL , 1.0f , 0 );

	immediateContext->IASetInputLayout( inputLayout );
	immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	UINT stride = sizeof( CustomVertex );
	UINT offset = 0;
	immediateContext->IASetVertexBuffers( 0 , 1 , &waveVB , &stride , &offset );
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
		{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0}
	};
	
	effectTech = effect->GetTechniqueByName( "SimpleTech" );
	D3DX11_PASS_DESC passDesc;
	effectTech->GetPassByIndex( 0 )->GetDesc( &passDesc );

	HR( device->CreateInputLayout( descList , 2 , passDesc.pIAInputSignature , passDesc.IAInputSignatureSize , &inputLayout ) );
}

void WaveScene::CreateWavesBuffer( const CustomVertex *vertices , UINT vertexNum , const UINT *indices , UINT indexNum )
{
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof( CustomVertex ) * vertexNum;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = vertices;

	HR( device->CreateBuffer( &bufferDesc , &initData , &waveVB ) );

	D3D11_BUFFER_DESC bufferDesc2;
	bufferDesc2.ByteWidth = sizeof( UINT ) * indexNum;
	bufferDesc2.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc2.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc2.CPUAccessFlags = 0;
	bufferDesc2.MiscFlags = 0;
	bufferDesc2.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData2;
	initData2.pSysMem = indices;

	HR( device->CreateBuffer( &bufferDesc2 , &initData2 , &waveIB ) );
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
	CreateWavesBuffer( &vlist[0] , vlist.size() , &ilist[0] , ilist.size() );
}
