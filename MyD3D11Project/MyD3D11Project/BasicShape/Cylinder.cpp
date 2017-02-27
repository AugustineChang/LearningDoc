#include "Cylinder.h"
#include "../Utilities/CommonHeader.h"
using namespace DirectX;


Cylinder::Cylinder() : topRadius( 0.8f ) , bottomRadius( 1.2f ) , height( 2.0f ) ,
	stackCount( 10 ) , sliceCount( 12 )
{
	createObjectMesh();
}


Cylinder::~Cylinder()
{
}

void Cylinder::createObjectMesh()
{
	//side vertex
	float deltaRadius = ( topRadius - bottomRadius ) / ( stackCount - 1 );
	float deltaHeight = height / ( stackCount - 1 );
	float halfHeight = height * 0.5f;
	for ( UINT i = 0; i < stackCount; ++i )
	{
		generateCicle( bottomRadius + deltaRadius*i , deltaHeight*i - halfHeight );
	}

	//top vertex
	CustomVertex top;
	top.Pos = XMFLOAT3( 0.0f , halfHeight , 0.0f );
	top.Color = XMFLOAT4( 0.0f , 0.0f , 1.0f , 1.0f );
	vertices.push_back( top );

	//bottom vertex
	CustomVertex bottom;
	bottom.Pos = XMFLOAT3( 0.0f , -halfHeight , 0.0f );
	bottom.Color = XMFLOAT4( 0.0f , 1.0f , 0.0f , 1.0f );
	vertices.push_back( bottom );

	//side index
	for ( UINT circle = 0; circle < stackCount - 1; ++circle )
	{
		for ( UINT vert = 0; vert < sliceCount; ++vert )
		{
			UINT curIndex = circle*( sliceCount + 1 ) + vert;
			UINT nextIndex = curIndex + 1;
			UINT nextCircleIndex = curIndex + sliceCount + 1;
			UINT nextCircleNextIndex = nextCircleIndex + 1;

			// up triangle
			indices.push_back( curIndex );
			indices.push_back( nextCircleIndex );
			indices.push_back( nextCircleNextIndex );

			// down triangle
			indices.push_back( curIndex );
			indices.push_back( nextCircleNextIndex );
			indices.push_back( nextIndex );
		}
	}

	//top index
	UINT topIndex = vertices.size() - 2;
	for ( UINT vert = 0; vert < sliceCount; ++vert )
	{
		UINT curIndex = ( stackCount - 1 )*( sliceCount + 1 ) + vert;
		UINT nextIndex = curIndex + 1;

		indices.push_back( curIndex );
		indices.push_back( topIndex );
		indices.push_back( nextIndex );
	}

	//bottom index
	UINT bottomIndex = vertices.size() - 1;
	for ( UINT vert = 0; vert < sliceCount; ++vert )
	{
		UINT curIndex = vert;
		UINT nextIndex = curIndex + 1;

		indices.push_back( curIndex );
		indices.push_back( nextIndex );
		indices.push_back( bottomIndex );
	}
}

void Cylinder::generateCicle( float radius , float yPos )
{
	float deltaAngle = 2.0f * SimpleMath::PI / sliceCount;
	float halfHeight = height * 0.5f;

	for ( UINT i = 0; i < sliceCount; ++i )
	{
		CustomVertex one;
		one.Pos = XMFLOAT3( radius * cosf( deltaAngle*i ) , yPos , radius * sinf( deltaAngle*i ) );
		one.Color = XMFLOAT4( 0.0f , 1.0f - ( yPos + halfHeight ) / height , ( yPos + halfHeight ) / height , 1.0f );
		vertices.push_back( one );
	}

	//duplicate first
	UINT size = vertices.size();
	vertices.push_back( vertices[size - sliceCount] );
}
