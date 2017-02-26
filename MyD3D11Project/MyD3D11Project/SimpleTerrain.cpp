#include "SimpleTerrain.h"
#include "CommonHeader.h"
using namespace DirectX;



SimpleTerrain::SimpleTerrain()
	:verticesDim( { 50,50 } ) , terrainSize( { 160.0f,160.0f } )
{
	createObjectMesh();
}


SimpleTerrain::~SimpleTerrain()
{
}

void SimpleTerrain::createObjectMesh()
{
	UINT vertexNum = verticesDim.x * verticesDim.y;
	UINT triangleNum = 2 * ( verticesDim.x - 1 )*( verticesDim.y - 1 );

	float gridX = terrainSize.x / ( verticesDim.x - 1 );
	float gridY = terrainSize.y / ( verticesDim.y - 1 );
	float halfSizeX = terrainSize.x * 0.5f;
	float halfSizeY = terrainSize.y * 0.5f;

	//generate Vertices
	for ( UINT z = 0; z < verticesDim.y; ++z )
	{
		for ( UINT x = 0; x < verticesDim.x; ++x )
		{
			CustomVertex one;

			float yPos = 0.3f*( z*sinf( 0.1f*x ) + x*cosf( 0.1f*z ) );

			one.Pos = XMFLOAT3( x * gridX - halfSizeX , yPos , z * gridY - halfSizeY );
			one.Color = XMFLOAT4( x * gridX / terrainSize.x , 0.0f , z * gridY / terrainSize.y , 1.0f );

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
