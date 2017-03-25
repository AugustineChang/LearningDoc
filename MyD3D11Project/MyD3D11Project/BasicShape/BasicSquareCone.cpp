#include "BasicSquareCone.h"
#include "DDSTextureLoader.h"
#include "../Utilities/CommonHeader.h"
using namespace DirectX;


BasicSquareCone::BasicSquareCone() : BasicShape() , rotSpeed( 1.0f )
{
	createObjectMesh();
}


BasicSquareCone::~BasicSquareCone()
{
}

void BasicSquareCone::UpdateObject( float DeltaTime )
{
	Rotation.y += rotSpeed * DeltaTime;

	float twoPI = SimpleMath::PI * 2.0f;
	if ( Rotation.y > twoPI ) Rotation.y -= twoPI;
}

void BasicSquareCone::createObjectTexture( struct ID3D11Device *device )
{
	CreateDDSTextureFromFile( device , L"Textures/WoodCrate02.dds" , &texture , &textureView );
}

void BasicSquareCone::createObjectMesh()
{
	XMFLOAT3 zero = XMFLOAT3( 0.0f , 0.0f , 0.0f );

	vertices =
	{
		{ XMFLOAT3( 1.0f,0.0f,1.0f ), zero , XMFLOAT2( 1.0f,0.0f ) },
		{ XMFLOAT3( -1.0f,0.0f,1.0f ), zero , XMFLOAT2( 0.0f,0.0f ) },
		{ XMFLOAT3( 0.0f,2.0f,0.0f ), zero , XMFLOAT2( 0.5f,1.0f ) }, // forward

		{ XMFLOAT3( -1.0f,0.0f,-1.0f ), zero , XMFLOAT2( 0.0f,0.0f ) },
		{ XMFLOAT3( 1.0f,0.0f,-1.0f ), zero , XMFLOAT2( 1.0f,0.0f ) },
		{ XMFLOAT3( 0.0f,2.0f,0.0f ), zero , XMFLOAT2( 0.5f,1.0f ) }, // backward

		{ XMFLOAT3( -1.0f,0.0f,-1.0f ), zero , XMFLOAT2( 0.0f,0.0f ) },
		{ XMFLOAT3( -1.0f,0.0f,1.0f ), zero , XMFLOAT2( 1.0f,0.0f ) },
		{ XMFLOAT3( 0.0f,2.0f,0.0f ), zero , XMFLOAT2( 0.5f,1.0f ) }, // left

		{ XMFLOAT3( 1.0f,0.0f,-1.0f ), zero , XMFLOAT2( 0.0f,0.0f ) },
		{ XMFLOAT3( 1.0f,0.0f,1.0f ), zero , XMFLOAT2( 1.0f,0.0f ) },
		{ XMFLOAT3( 0.0f,2.0f,0.0f ), zero , XMFLOAT2( 0.5f,1.0f ) }, // right

		{ XMFLOAT3( 1.0f,0.0f,1.0f ), zero , XMFLOAT2( 1.0f,0.0f ) },
		{ XMFLOAT3( -1.0f,0.0f,1.0f ), zero , XMFLOAT2( 0.0f,0.0f ) },
		{ XMFLOAT3( -1.0f,0.0f,-1.0f ), zero , XMFLOAT2( 0.0f,1.0f ) },
		{ XMFLOAT3( 1.0f,0.0f,-1.0f ), zero , XMFLOAT2( 1.0f,1.0f ) } // down
	};

	indices =
	{
		0, 2, 1, // forward
		3, 5, 4, // back
		6, 7, 8, // left
		9, 11, 10, // right
		12, 13, 14,
		12, 14, 15 // down
	};

	computeNormal();
}
