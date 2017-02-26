#include "SimpleScene.h"
#include "CommonHeader.h"
#include <fstream>
#include <vector>
using namespace DirectX;


SimpleScene::SimpleScene( HINSTANCE hinstance , int show )
	:DirectXApp( hinstance , show ) , moveSpeed( 0.1f )
{
	lastMousePos.x = 0;
	lastMousePos.y = 0;

	squareCone.Position.x = 2.5f;
}


SimpleScene::~SimpleScene()
{
	ReleaseCOM( vertexBuffer );
	ReleaseCOM( indexBuffer );
	ReleaseCOM( inputLayout );
	ReleaseCOM( effect );
	ReleaseCOM( rasterState );
}

bool SimpleScene::InitDirectApp()
{
	if ( !DirectXApp::InitDirectApp() ) return false;

	createEffectAtBuildtime();
	createInputLayout();
	createObjects();

	camera.buildProjectMatrix( screenWidth , screenHeight );
	return true;
}

void SimpleScene::UpdateScene( float deltaTime )
{
}

void SimpleScene::DrawScene()
{
	immediateContext->ClearRenderTargetView( backBufferView , reinterpret_cast<const float *>( &XMVectorSet( 0.2f , 0.2f , 0.2f , 1.0f ) ) );
	immediateContext->ClearDepthStencilView( depthBufferView , D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL , 1.0f , 0 );

	immediateContext->IASetInputLayout( inputLayout );
	immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	UINT stride = sizeof( CustomVertex );
	UINT offset = 0;
	immediateContext->IASetVertexBuffers( 0 , 1 , &vertexBuffer , &stride , &offset );
	immediateContext->IASetIndexBuffer( indexBuffer , DXGI_FORMAT_R32_UINT , 0 );

	immediateContext->RSSetState( rasterState );
	
	cube.buildWorldMatrix();
	squareCone.buildWorldMatrix();
	camera.buildViewMatrix();

	UINT indexSize = cube.getIndices().size();
	UINT indexStart = 0;
	UINT indexBase = 0;
	renderObject( cube , indexSize , indexStart , indexBase );

	indexStart = indexSize;
	indexBase = cube.getVertices().size();
	indexSize = squareCone.getIndices().size();
	renderObject( squareCone , indexSize , indexStart , indexBase );

	HR( swapChain->Present( 0 , 0 ) );
}

void SimpleScene::OnResize()
{
	DirectXApp::OnResize();

	camera.buildProjectMatrix( screenWidth , screenHeight );
}

void SimpleScene::OnMouseDown( WPARAM btnState , int x , int y )
{
	lastMousePos.x = x;
	lastMousePos.y = y;

	SetCapture( ghMainWnd );
}

void SimpleScene::OnMouseMove( WPARAM btnState , int x , int y )
{
	if ( ( btnState & MK_RBUTTON ) != 0 )
	{
		float deltaX = XMConvertToRadians( ( x - lastMousePos.x )*moveSpeed );
		float deltaY = XMConvertToRadians( ( y - lastMousePos.y )*moveSpeed );
		
		camera.Rotation.y += deltaX;
		camera.Rotation.x = SimpleMath::Clamp<float>( camera.Rotation.x + deltaY , -SimpleMath::PI / 2 + 0.01f , SimpleMath::PI / 2 - 0.01f );

		camera.Position.x = 5.0f * cosf( camera.Rotation.x ) * cosf( -camera.Rotation.y - SimpleMath::PI / 2 );
		camera.Position.z = 5.0f * cosf( camera.Rotation.x ) * sinf( -camera.Rotation.y - SimpleMath::PI / 2 );
		camera.Position.y = 5.0f * sinf( camera.Rotation.x );

		lastMousePos.x = x;
		lastMousePos.y = y;
	}
}

void SimpleScene::OnMouseUp( WPARAM btnState , int x , int y )
{
	ReleaseCapture();
}

void SimpleScene::createInputLayout()
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

void SimpleScene::createVertexBuffer( const CustomVertex *vertices , UINT vertexNum )
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
	
	HR( device->CreateBuffer( &bufferDesc , &initData , &vertexBuffer ) );
}

void SimpleScene::createIndexBuffer( const UINT *indices , UINT indexNum )
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

void SimpleScene::renderObject( const BasicShape &basicObj , UINT indexSize , UINT indexStart, UINT indexBase )
{
	XMMATRIX tempW = basicObj.getWorldMatrix();
	XMMATRIX tempV = camera.getViewMatrix();
	XMMATRIX tempP = camera.getProjectMatrix();
	XMMATRIX tempWVP = tempW * tempV * tempP;
	effectWVP->SetMatrix( reinterpret_cast<float*>( &tempWVP ) );

	D3DX11_TECHNIQUE_DESC techDesc;
	effectTech->GetDesc( &techDesc );
	for ( UINT i = 0; i < techDesc.Passes; ++i )
	{
		effectTech->GetPassByIndex( i )->Apply( 0 , immediateContext );
		
		immediateContext->DrawIndexed( indexSize , indexStart , indexBase );
	}
}

void SimpleScene::createRenderState()
{
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory( &rsDesc , sizeof( D3D11_RASTERIZER_DESC ) );
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;
	
	device->CreateRasterizerState( &rsDesc , &rasterState );
}

void SimpleScene::createEffectAtRuntime()
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

void SimpleScene::createEffectAtBuildtime()
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

void SimpleScene::createObjects()
{
	std::vector<CustomVertex> vlist1 = cube.getVertices();
	std::vector<UINT> ilist1 = cube.getIndices();

	std::vector<CustomVertex> vlist2 = squareCone.getVertices();
	std::vector<UINT> ilist2 = squareCone.getIndices();

	std::vector<CustomVertex> gvlist;
	for ( CustomVertex vertex : vlist1 )
	{
		gvlist.push_back( vertex );
	}
	for ( CustomVertex vertex : vlist2 )
	{
		gvlist.push_back( vertex );
	}

	std::vector<UINT> gilist;
	for ( UINT index : ilist1 )
	{
		gilist.push_back( index );
	}
	for ( UINT index : ilist2 )
	{
		gilist.push_back( index );
	}

	createVertexBuffer( &gvlist[0] , gvlist.size() );
	createIndexBuffer( &gilist[0] , gilist.size() );
}
