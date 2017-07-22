#include "SimpleScene.h"

#include "../Utilities/CommonHeader.h"
#include "../BasicShape/BasicCube.h"
#include "../BasicShape/BasicSquareCone.h"
#include "../BasicShape/SimpleTerrain.h"
#include "../BasicShape/Cylinder.h"
#include "../BasicShape/Sphere.h"
#include "../BasicShape/GeoSphere.h"
#include "../BasicShape/SimpleMesh.h"
#include "../BasicShape/RotateFlame.h"
#include "../BasicShape/BlendFlame.h"
#include "../BasicShape/Billboard.h"
#include "../BasicShape/BasicCurve.h"
#include "../BasicShape/InstancedCube.h"

using namespace DirectX;


SimpleScene::SimpleScene( HINSTANCE hinstance , int show )
	:DirectXApp( hinstance , show ) , moveSpeed( 0.1f ) , 
	radius( 5.0f ) , zoomSpeed( 0.005f )
{
	lastMousePos.x = 0;
	lastMousePos.y = 0;

	BasicSquareCone *squareCone = new BasicSquareCone();
	squareCone->Position.z = 3.0f;
	renderList.push_back( squareCone );

	Cylinder *cylinder = new Cylinder();
	cylinder->Position.y = 2.1f;
	renderList.push_back( cylinder );

	Sphere *sphere = new Sphere();
	sphere->Position.x = 3;
	renderList.push_back( sphere );

	BasicCube *cube = new BasicCube();
	renderList.push_back( cube );

	//SimpleMesh *mesh = new SimpleMesh();
	//mesh->Position.y = 1.5f;
	//renderList.push_back( mesh );

	InstancedCube *instCube = new InstancedCube();
	instCube->Position.x = -2;
	renderList.push_back( instCube );

	BasicCurve *curve = new BasicCurve();
	curve->Position.y = 3.5f;
	renderList.push_back( curve );

	DirectX::XMVECTOR dir = DirectX::XMVectorSet( -1.0f , 1.0f , 0.5f , 0.0f );
	DirectX::XMStoreFloat3( &dirLight[1].direction , DirectX::XMVector3Normalize( dir ) );

	dirLight[1].diffuseColor = DirectX::XMFLOAT4( 0.0f , 0.8f , 0.8f , 1.0f );
}


SimpleScene::~SimpleScene()
{
	ReleaseCOM( vertexBuffer );
	ReleaseCOM( indexBuffer );

	for ( BasicShape *shape : renderList )
	{
		delete shape;
	}
	renderList.clear();
}

bool SimpleScene::InitDirectApp()
{
	if ( !DirectXApp::InitDirectApp() ) return false;
	
	createObjects();

	camera.Position.z = -radius;
	camera.buildProjectMatrix( screenWidth , screenHeight );
	return true;
}

void SimpleScene::UpdateScene( float deltaTime )
{
	for ( BasicShape *shape : renderList )
	{
		shape->doFrustumCull( &camera );
		shape->UpdateObject( deltaTime , immediateContext );
	}
}

void SimpleScene::DrawScene()
{
	immediateContext->ClearRenderTargetView( backBufferView , reinterpret_cast<const float *>( &XMVectorSet( 0.2f , 0.2f , 0.2f , 1.0f ) ) );
	immediateContext->ClearDepthStencilView( depthBufferView , D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL , 1.0f , 0 );

	camera.buildViewMatrix();
	
	for ( BasicShape *shape : renderList )
	{
		if ( !shape->isPassFrustumTest ) continue;

		shape->UpdateDirectionalLight( &dirLight[0] , 2 );
		shape->UpdateObjectEffect( &camera );

		switch ( shape->type )
		{
			case ShapeType::Standard:
			{
				UINT stride = sizeof( BaseVertex );
				UINT offset = 0;
				immediateContext->IASetVertexBuffers( 0 , 1 , &vertexBuffer , &stride , &offset );
				immediateContext->IASetIndexBuffer( indexBuffer , DXGI_FORMAT_R32_UINT , 0 );
			}
			break;

			case ShapeType::Instanced:
			{
				UINT stride[2] = { sizeof( BaseVertex ), sizeof( InstanceData ) };
				UINT offset[2] = { 0,0 };
				ID3D11Buffer *instanceData = shape->getInstancedData();
				ID3D11Buffer *buffers[2] = { vertexBuffer, instanceData };

				immediateContext->IASetVertexBuffers( 0 , 2 , buffers , stride , offset );
				immediateContext->IASetIndexBuffer( indexBuffer , DXGI_FORMAT_R32_UINT , 0 );
			}
			break;

			case ShapeType::Custom:
			break;
		}
		
		shape->RenderObject( immediateContext );
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
		
		camera.RotateCamera( deltaX , deltaY );
		camera.MoveCamera_Orbit( radius );

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
	camera.MoveCamera_Orbit( radius );
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

void SimpleScene::createDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC dssDesc;
	dssDesc.DepthEnable = false;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dssDesc.StencilEnable = false;
	dssDesc.StencilReadMask = 0xff;
	dssDesc.StencilWriteMask = 0xff;

	dssDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dssDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dssDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dssDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	dssDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dssDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dssDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dssDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HR( device->CreateDepthStencilState( &dssDesc , &depthState ) );
}

void SimpleScene::createObjects()
{
	if ( renderList.size() <= 0 ) return;

	std::vector<BaseVertex> gvlist;
	std::vector<UINT> gilist;
	for ( BasicShape *shape : renderList )
	{
		shape->InitShape( device );

		switch ( shape->type )
		{
		case ShapeType::Standard:
			addToGlobalBuffer( gvlist , gilist , *shape );
			break;

		case ShapeType::Instanced:
			addToGlobalBuffer( gvlist , gilist , *shape );
			break;

		case ShapeType::Custom:
			break;
		}
	}

	createVertexBuffer( &gvlist[0] , gvlist.size() );
	createIndexBuffer( &gilist[0] , gilist.size() );
}

void SimpleScene::addToGlobalBuffer( std::vector<BaseVertex> &gVBuffer , std::vector<UINT> &gIBuffer , BasicShape &shape )
{
	std::vector<BaseVertex> vlist = shape.getVertices();
	std::vector<UINT> ilist = shape.getIndices();

	shape.indexSize = ilist.size();
	shape.indexStart = gIBuffer.size();
	shape.indexBase = gVBuffer.size();

	gVBuffer.insert( gVBuffer.end() , vlist.begin() , vlist.end() );
	gIBuffer.insert( gIBuffer.end() , ilist.begin() , ilist.end() );
}
