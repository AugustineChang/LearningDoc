#include "Sphere.h"
#include "DDSTextureLoader.h"
#include "../Utilities/CommonHeader.h"
using namespace DirectX;


Sphere::Sphere() : stackCount( 17 ) , sliceCount( 20 ) , radius( 2.0f )
{
	material.specular.w = 3.0f;
}


Sphere::~Sphere()
{
}


void Sphere::createObjectMesh()
{
	//side vertex
	float deltaVAngle = SimpleMath::PI / ( stackCount - 1 );
	for ( UINT i = 1; i < stackCount - 1; ++i )
	{
		float vAngle = deltaVAngle*i;
		generateCicle( vAngle );
	}

	//top vertex
	CustomVertex top;
	top.Pos = XMFLOAT3( 0.0f , radius , 0.0f );
	top.Normal = XMFLOAT3( 0.0f , 0.0f , 0.0f );
	top.TexCoord = XMFLOAT2( 0.0f , 0.0f );
	vertices.push_back( top );

	//bottom vertex
	CustomVertex bottom;
	bottom.Pos = XMFLOAT3( 0.0f , -radius , 0.0f );
	bottom.Normal = XMFLOAT3( 0.0f , 0.0f , 0.0f );
	bottom.TexCoord = XMFLOAT2( 0.0f , 1.0f );
	vertices.push_back( bottom );

	//side index
	for ( UINT circle = 0; circle < stackCount - 3; ++circle )
	{
		for ( UINT vert = 0; vert < sliceCount; ++vert )
		{
			UINT curIndex = circle*( sliceCount + 1 ) + vert;
			UINT nextIndex = curIndex + 1;
			UINT nextCircleIndex = curIndex + sliceCount + 1;
			UINT nextCircleNextIndex = nextCircleIndex + 1;

			// up triangle
			indices.push_back( curIndex );
			indices.push_back( nextIndex );
			indices.push_back( nextCircleIndex );

			// down triangle
			indices.push_back( nextIndex );
			indices.push_back( nextCircleNextIndex );
			indices.push_back( nextCircleIndex );
		}
	}

	//top index
	UINT topIndex = vertices.size() - 2;
	for ( UINT vert = 0; vert < sliceCount; ++vert )
	{
		UINT curIndex = vert;
		UINT nextIndex = curIndex + 1;

		indices.push_back( curIndex );
		indices.push_back( topIndex );
		indices.push_back( nextIndex );
	}

	//bottom index
	UINT bottomIndex = vertices.size() - 1;
	for ( UINT vert = 0; vert < sliceCount; ++vert )
	{
		UINT curIndex = ( stackCount - 3 )*( sliceCount + 1 ) + vert;
		UINT nextIndex = curIndex + 1;

		indices.push_back( curIndex );
		indices.push_back( nextIndex );
		indices.push_back( bottomIndex );
	}

	computeNormal();
}

void Sphere::createObjectTexture( struct ID3D11Device *device )
{
	CreateDDSTextureFromFile( device , L"Textures/WoodCrate01.dds" , &texture , &textureView );
}

void Sphere::generateCicle( float vAngle )
{
	float deltaHAngle = 2.0f * SimpleMath::PI / sliceCount;
	XMFLOAT3 zero = XMFLOAT3( 0.0f , 0.0f , 0.0f );

	for ( UINT i = 0; i < sliceCount; ++i )
	{
		CustomVertex one;

		float hAngle = deltaHAngle*i;

		one.Pos = XMFLOAT3( radius * sinf( vAngle ) * cosf( hAngle ) ,
						   radius * cosf( vAngle ) ,
						   radius * sinf( vAngle ) * sinf( hAngle ) );
		one.Normal = zero;
		one.TexCoord = SimpleMath::Div( XMFLOAT2( hAngle , vAngle ) , XMFLOAT2( 2.0f*SimpleMath::PI , SimpleMath::PI ) );

		vertices.push_back( one );
	}

	//duplicate first
	UINT size = vertices.size();
	vertices.push_back( vertices[size - sliceCount] );
}