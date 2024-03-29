#include "SimpleTerrain.h"
#include "DDSTextureLoader.h"
#include "../Utilities/CommonHeader.h"
using namespace DirectX;



SimpleTerrain::SimpleTerrain()
	:verticesDim( { 50,50 } ) , terrainSize( { 160.0f,160.0f } )
{
	material->specular = XMFLOAT4( 0.0f , 0.0f , 0.0f , 1.0f );
}

SimpleTerrain::SimpleTerrain( const Point<unsigned int> &vert , const Point<float> &size )
	: verticesDim( vert ) , terrainSize( size )
{
}


SimpleTerrain::~SimpleTerrain()
{
}

void SimpleTerrain::createObjectTexture( struct ID3D11Device *device )
{
	CreateDDSTextureFromFile( device , L"Textures/grass.dds" , &texture , &textureView );
}

void SimpleTerrain::createObjectMesh()
{
	createBasicPlane();

	for ( BaseVertex &vertex : vertices )
	{
		vertex.Pos.y = getHeight( vertex.Pos.x , vertex.Pos.z );
	}

	computeNormal();
	computeBoundingBox();
}

void SimpleTerrain::createBasicPlane()
{
	//UINT vertexNum = verticesDim.x * verticesDim.y;
	//UINT triangleNum = 2 * ( verticesDim.x - 1 )*( verticesDim.y - 1 );

	float gridX = terrainSize.x / ( verticesDim.x - 1 );
	float gridY = terrainSize.y / ( verticesDim.y - 1 );
	float halfSizeX = terrainSize.x * 0.5f;
	float halfSizeY = terrainSize.y * 0.5f;

	XMFLOAT3 zero = XMFLOAT3( 0.0f , 0.0f , 0.0f );
	float du = 1.0f / verticesDim.x;
	float dv = 1.0f / verticesDim.y;

	//generate Vertices
	for ( UINT z = 0; z < verticesDim.y; ++z )
	{
		for ( UINT x = 0; x < verticesDim.x; ++x )
		{
			BaseVertex one;

			one.Pos = XMFLOAT3( x * gridX - halfSizeX , 0.0f , z * gridY - halfSizeY );
			one.Normal = zero;
			one.TexCoord = XMFLOAT2( x * du , z * dv );
			one.TangentU = XMFLOAT3( 1.0f , 0.0f , 0.0f );

			vertices.push_back( one );
		}
	}

	//generate Indices
	for ( UINT z = 0; z < verticesDim.y - 1; ++z )
	{
		for ( UINT x = 0; x < verticesDim.x - 1; ++x )
		{
			UINT curIndex = z*verticesDim.x + x;
			UINT nextIndex = curIndex + 1;
			UINT nextRowIndex = curIndex + verticesDim.x;
			UINT nextRowNextIndex = nextRowIndex + 1;

			//up triangle
			indices.push_back( curIndex );
			indices.push_back( nextRowIndex );
			indices.push_back( nextIndex );

			//down triangle
			indices.push_back( nextIndex );
			indices.push_back( nextRowIndex );
			indices.push_back( nextRowNextIndex );
		}
	}
}

void SimpleTerrain::computeBoundingBox()
{
	float halfSizeX = terrainSize.x * 0.5f;
	float halfSizeY = terrainSize.y * 0.5f;

	boundingBox.Center = XMFLOAT3( 0.0f , 0.0f , 0.0f );
	boundingBox.Extents = XMFLOAT3( halfSizeX , halfSizeY , halfSizeY );
}

float SimpleTerrain::getHeight( float x , float z ) const
{
	return 0.3f*( z*sinf( 0.1f*x ) + x*cosf( 0.1f*z ) );
}

void SimpleTerrain::createRenderState( ID3D11Device *device )
{
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory( &rsDesc , sizeof( D3D11_RASTERIZER_DESC ) );
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;

	device->CreateRasterizerState( &rsDesc , &rasterState );
}
