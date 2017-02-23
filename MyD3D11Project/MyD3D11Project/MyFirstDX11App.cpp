#include "MyFirstDX11App.h"
#include "CommonHeader.h"
#include "d3dx11effect.h"
using namespace DirectX;


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

void MyFirstDX11App::createInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC descList[] =
	{
		{"Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"Color",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0}
	};

	//device->CreateInputLayout(descList,2,)
}

void MyFirstDX11App::createVertexBuffer( const CustomVertex *vertices , UINT vertexNum )
{
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof( CustomVertex ) * vertexNum;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = vertices;

	ID3D11Buffer *vertexBuffer;
	HR( device->CreateBuffer( &bufferDesc , &initData , &vertexBuffer ) );

	UINT stride = sizeof( CustomVertex );
	UINT offset = 0;
	immediateContext->IASetVertexBuffers( 0 , 1 , &vertexBuffer , &stride , &offset );
}

void MyFirstDX11App::createIndexBuffer( const UINT *indices , UINT indexNum )
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

	ID3D11Buffer *indexBuffer;
	HR( device->CreateBuffer( &bufferDesc , &initData , &indexBuffer ) );

	immediateContext->IASetIndexBuffer( indexBuffer , DXGI_FORMAT_R32_UINT , 0 );
}

void MyFirstDX11App::setRenderState()
{
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory( &rsDesc , sizeof( D3D11_RASTERIZER_DESC ) );
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;

	ID3D11RasterizerState *rasterState;
	device->CreateRasterizerState( &rsDesc , &rasterState );

}

void MyFirstDX11App::createEffect()
{
	DWORD shaderFlag = 0;
#if defined(DEBUG) | defined(_DEBUG)
	shaderFlag |= D3D10_SHADER_DEBUG;
	shaderFlag |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	
	ID3D10Blob *compilationMsgs;
	ID3DX11Effect *effect;
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

	//ID3DX11EffectMatrixVariable* fxWVP;
	//fxWVP = effect->GetVariableByName( "gWVP" )->AsMatrix();
	//fxWVP->SetMatrix();
}

void MyFirstDX11App::createCube()
{
	XMFLOAT4 whiteFloat;
	XMStoreFloat4( &whiteFloat , Colors::White );
	XMFLOAT4 blackFloat;
	XMStoreFloat4( &whiteFloat , Colors::Black );
	XMFLOAT4 redFloat;
	XMStoreFloat4( &whiteFloat , Colors::Red );
	XMFLOAT4 greenFloat;
	XMStoreFloat4( &whiteFloat , Colors::Green );
	XMFLOAT4 blueFloat;
	XMStoreFloat4( &whiteFloat , Colors::Blue );
	XMFLOAT4 yellowFloat;
	XMStoreFloat4( &whiteFloat , Colors::Yellow );
	XMFLOAT4 cyanFloat;
	XMStoreFloat4( &whiteFloat , Colors::Cyan );
	XMFLOAT4 magentaFloat;
	XMStoreFloat4( &whiteFloat , Colors::Magenta );

	CustomVertex vertices[] =
	{
		{ XMFLOAT3( 1.0f,1.0f,1.0f ), whiteFloat },
		{ XMFLOAT3( -1.0f,1.0f,1.0f ), blackFloat },
		{ XMFLOAT3( 1.0f,1.0f,-1.0f ), redFloat },
		{ XMFLOAT3( -1.0f,1.0f,-1.0f ), greenFloat },
		{ XMFLOAT3( 1.0f,-1.0f,1.0f ), blueFloat },
		{ XMFLOAT3( -1.0f,-1.0f,1.0f ), yellowFloat },
		{ XMFLOAT3( 1.0f,-1.0f,-1.0f ), cyanFloat },
		{ XMFLOAT3( -1.0f,-1.0f,-1.0f ), magentaFloat }
	};
	createVertexBuffer( vertices , 8 );

	UINT indices[36] = 
	{
		0, 2, 1,
		0, 3, 1, // up
		0, 6, 2,
		0, 4, 6, // right
		0, 1, 5,
		0, 5, 4, // forward
		1, 3, 7,
		1, 7, 5, // left
		3, 2, 6,
		3, 6, 7, // back
		5, 7, 6,
		4, 5, 6  // down
	};
	createIndexBuffer( indices , 36 );
}
