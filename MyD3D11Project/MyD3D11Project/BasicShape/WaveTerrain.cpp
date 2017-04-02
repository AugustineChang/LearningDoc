#include <assert.h>
#include "WaveTerrain.h"
#include "DDSTextureLoader.h"
#include "../Utilities/CommonHeader.h"
using namespace DirectX;

WaveTerrain::WaveTerrain() : SimpleTerrain( { 100,100 } , { 160,160 } ) , dx( 0.8f ) , timeStep( 0.03f ) , speed( 3.25f ) , damping( 0.4f ) ,
timer( 0.0f ) , disturbTimer( 0.0f )
{
	float d = damping*timeStep + 2.0f;
	float e = ( speed*speed )*( timeStep*timeStep ) / ( dx*dx );
	K1 = ( damping*timeStep - 2.0f ) / d;
	K2 = ( 4.0f - 8.0f*e ) / d;
	K3 = ( 2.0f*e ) / d;

	material.diffuse.w = 0.4f;
}


WaveTerrain::~WaveTerrain()
{
}

void WaveTerrain::UpdateObject( float DeltaTime )
{
	UpdateDisturb( DeltaTime );

	timer += DeltaTime;
	if ( timer <= timeStep ) return;
	
	UINT numCol = verticesDim.y;

	for ( UINT i = 1; i < verticesDim.x - 1; ++i )
	{
		for ( UINT j = 1; j < verticesDim.y - 1; ++j )
		{
			prevVertice[i*numCol + j].Pos.y =
				K1*prevVertice[i*numCol + j].Pos.y +
				K2*vertices[i*numCol + j].Pos.y +
				K3*( vertices[( i + 1 )*numCol + j].Pos.y +
					vertices[( i - 1 )*numCol + j].Pos.y +
					vertices[i*numCol + j + 1].Pos.y +
					vertices[i*numCol + j - 1].Pos.y );
		}
	}

	std::swap( prevVertice , vertices );
	computeNormal();

	timer = 0.0f;
}

void WaveTerrain::UpdateDisturb( float DeltaTime )
{
	disturbTimer += DeltaTime;
	if ( disturbTimer <= 0.25f )return;

	UINT i = 5 + rand() % 90;
	UINT j = 5 + rand() % 90;

	float r = SimpleMath::RandF( 0.5f , 1.0f );

	disturb( i , j , r );
	disturbTimer = 0.0f;
}

float WaveTerrain::getHeight( float x , float z , float time ) const
{
	return 0.0f;
}

void WaveTerrain::computeNormal()
{
	UINT numRow = verticesDim.x;
	for ( UINT i = 0; i < verticesDim.y; ++i )
	{
		for ( UINT j = 0; j < verticesDim.x; ++j )
		{
			UINT curIndex = j + i * numRow;
			UINT nextIndex = ( j == verticesDim.x - 1 ) ? curIndex - 1 : curIndex + 1;
			UINT nextRowIndex = ( i == verticesDim.y - 1 ) ? curIndex - numRow : curIndex + numRow;

			XMVECTOR curPoint = XMLoadFloat3( &vertices[curIndex].Pos );
			XMVECTOR nextPoint = XMLoadFloat3( &vertices[nextIndex].Pos );
			XMVECTOR nextRowPoint = XMLoadFloat3( &vertices[nextRowIndex].Pos );

			XMVECTOR deltaX = nextPoint - curPoint;
			XMVECTOR deltaY = nextRowPoint - curPoint;

			XMVECTOR normal = XMVector3Cross( deltaY , deltaX );
			XMStoreFloat3( &vertices[curIndex].Normal , XMVector3Normalize( normal ) );
		}
	}
}

void WaveTerrain::createObjectMesh()
{
	createBasicPlane();

	UINT len = vertices.size();
	for ( const CustomVertex &vert : vertices )
	{
		prevVertice.push_back( vert );
	}
}

void WaveTerrain::createObjectTexture( struct ID3D11Device *device )
{
	CreateDDSTextureFromFile( device , L"Textures/water1.dds" , &texture , &textureView );
}

void WaveTerrain::createBlendState( ID3D11Device *device )
{
	D3D11_BLEND_DESC blendDesc = { 0 };
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;

	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR( device->CreateBlendState( &blendDesc , &blendState ) );
}

void WaveTerrain::disturb( unsigned int i , unsigned int j , float magnitude )
{
	assert( i > 1 && i < verticesDim.x - 2 );
	assert( j > 1 && j < verticesDim.y - 2 );

	float halfMag = 0.5f*magnitude;
	UINT numCol = verticesDim.y;

	// Disturb the ijth vertex height and its neighbors.
	vertices[i*numCol + j].Pos.y += magnitude;
	vertices[i*numCol + j + 1].Pos.y += halfMag;
	vertices[i*numCol + j - 1].Pos.y += halfMag;
	vertices[( i + 1 )*numCol + j].Pos.y += halfMag;
	vertices[( i - 1 )*numCol + j].Pos.y += halfMag;
}
