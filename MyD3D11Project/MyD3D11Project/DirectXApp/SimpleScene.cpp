#include "SimpleScene.h"
#include "../Utilities/CommonHeader.h"
#include "../BasicShape/BasicCube.h"
#include "../BasicShape/BasicSquareCone.h"
#include "../BasicShape/SimpleTerrain.h"
#include "../BasicShape/Cylinder.h"
#include "../BasicShape/Sphere.h"
#include "../BasicShape/GeoSphere.h"
#include "../BasicShape/SimpleMesh.h"
using namespace DirectX;


SimpleScene::SimpleScene( HINSTANCE hinstance , int show )
	:DirectXApp( hinstance , show ) , moveSpeed( 0.1f ) , 
	radius( 5.0f ) , zoomSpeed( 0.005f )
{
	lastMousePos.x = 0;
	lastMousePos.y = 0;

	BasicCube *cube = new BasicCube();
	renderList.push_back( cube );

	BasicSquareCone *squareCone = new BasicSquareCone();
	squareCone->Position.z = 3.0f;
	renderList.push_back( squareCone );

	//SimpleTerrain *terrain = new SimpleTerrain();
	//renderList.push_back( terrain );

	Cylinder *cylinder = new Cylinder();
	cylinder->Position.y = 2.0f;
	renderList.push_back( cylinder );

	//GeoSphere *sphere = new GeoSphere();
	//renderList.push_back( sphere );

	Sphere *sphere = new Sphere();
	sphere->Position.x = 3;
	renderList.push_back( sphere );

	//SimpleMesh *mesh = new SimpleMesh();
	//mesh->Position.y = 1.5f;
	//renderList.push_back( mesh );
}


SimpleScene::~SimpleScene()
{
	ReleaseCOM( vertexBuffer );
	ReleaseCOM( indexBuffer );
	ReleaseCOM( inputLayout );
	ReleaseCOM( rasterState );

	for ( BasicShape *shape : renderList )
	{
		delete shape;
	}
	renderList.clear();
}

bool SimpleScene::InitDirectApp()
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

void SimpleScene::UpdateScene( float deltaTime )
{
	for ( BasicShape *shape : renderList )
	{
		shape->UpdateObject( deltaTime );
	}

	/*rotAngle += deltaTime * 0.1f;
	rotMatrix = XMMatrixTranslation( -0.5f , -0.5f , 0.0f ) * 
		XMMatrixRotationZ( rotAngle ) * XMMatrixTranslation( 0.5f , 0.5f , 0.0f );*/
}

void SimpleScene::DrawScene()
{
	immediateContext->ClearRenderTargetView( backBufferView , reinterpret_cast<const float *>( &XMVectorSet( 0.2f , 0.2f , 0.2f , 1.0f ) ) );
	immediateContext->ClearDepthStencilView( depthBufferView , D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL , 1.0f , 0 );

	immediateContext->IASetInputLayout( inputLayout );
	immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	if ( renderList.size() > 0 )
	{
		UINT stride = sizeof( CustomVertex );
		UINT offset = 0;
		immediateContext->IASetVertexBuffers( 0 , 1 , &vertexBuffer , &stride , &offset );
		immediateContext->IASetIndexBuffer( indexBuffer , DXGI_FORMAT_R32_UINT , 0 );
	}
	immediateContext->RSSetState( rasterState );
	
	for ( BasicShape *shape : renderList )
	{
		shape->buildWorldMatrix();
	}
	camera.buildViewMatrix();
	effect.UpdateSceneEffect( &camera , &dirLight , &pointLight , &spotLight );

	for ( BasicShape *shape : renderList )
	{
		renderObject( *shape , shape->indexSize , shape->indexStart , shape->indexBase );
	}

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

		camera.Position.x = radius * cosf( camera.Rotation.x ) * cosf( -camera.Rotation.y - SimpleMath::PI / 2 );
		camera.Position.z = radius * cosf( camera.Rotation.x ) * sinf( -camera.Rotation.y - SimpleMath::PI / 2 );
		camera.Position.y = radius * sinf( camera.Rotation.x );

		lastMousePos.x = x;
		lastMousePos.y = y;
	}
}

void SimpleScene::OnMouseUp( WPARAM btnState , int x , int y )
{
	ReleaseCapture();
}

void SimpleScene::OnMouseWheel( int zDelta )
{
	radius -= zDelta * zoomSpeed;

	camera.Position.x = radius * cosf( camera.Rotation.x ) * cosf( -camera.Rotation.y - SimpleMath::PI / 2 );
	camera.Position.z = radius * cosf( camera.Rotation.x ) * sinf( -camera.Rotation.y - SimpleMath::PI / 2 );
	camera.Position.y = radius * sinf( camera.Rotation.x );
}

void SimpleScene::createInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC descList[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};
	
	D3DX11_PASS_DESC passDesc;
	effect.getEffectTech()->GetPassByIndex( 0 )->GetDesc( &passDesc );

	HR( device->CreateInputLayout( descList , 3 , passDesc.pIAInputSignature , passDesc.IAInputSignatureSize , &inputLayout ) );
}

template<typename T>
void SimpleScene::createVertexBuffer( const T *vertices , UINT vertexNum )
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
	XMMATRIX &tempW = basicObj.getWorldMatrix();
	XMMATRIX &tempV = camera.getViewMatrix();
	XMMATRIX &tempP = camera.getProjectMatrix();
	XMMATRIX tempWVP = tempW * tempV * tempP;

	XMMATRIX inverseW = XMMatrixInverse( &XMMatrixDeterminant( tempW ) , tempW );
	XMMATRIX inverseTransposeW = XMMatrixTranspose( inverseW );
	XMMATRIX identityMat = XMMatrixIdentity();

	effect.UpdateObjectEffect( tempWVP , tempW , inverseTransposeW , identityMat , &basicObj );

	D3DX11_TECHNIQUE_DESC techDesc;
	effect.getEffectTech()->GetDesc( &techDesc );
	for ( UINT i = 0; i < techDesc.Passes; ++i )
	{
		effect.getEffectTech()->GetPassByIndex( i )->Apply( 0 , immediateContext );
		
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

void SimpleScene::createObjects()
{
	if ( renderList.size() <= 0 ) return;

	std::vector<CustomVertex> gvlist;
	std::vector<UINT> gilist;
	for ( BasicShape *shape : renderList )
	{
		createGlobalBuffer( gvlist , gilist , *shape );

		shape->createObjectTexture( device );
	}

	createVertexBuffer( &gvlist[0] , gvlist.size() );
	createIndexBuffer( &gilist[0] , gilist.size() );
}

void SimpleScene::createGlobalBuffer( std::vector<CustomVertex> &gVBuffer , std::vector<UINT> &gIBuffer , BasicShape &shape )
{
	std::vector<CustomVertex> vlist = shape.getVertices();
	std::vector<UINT> ilist = shape.getIndices();

	shape.indexSize = ilist.size();
	shape.indexStart = gIBuffer.size();
	shape.indexBase = gVBuffer.size();

	gVBuffer.insert( gVBuffer.end() , vlist.begin() , vlist.end() );
	gIBuffer.insert( gIBuffer.end() , ilist.begin() , ilist.end() );
}
