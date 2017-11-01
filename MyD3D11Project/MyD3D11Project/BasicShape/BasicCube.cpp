#include "BasicCube.h"
#include "../Utilities/CommonHeader.h"
#include "../DirectXApp/Lights.h"
#include "DDSTextureLoader.h"
#include <sstream>
using namespace DirectX;


BasicCube::BasicCube()
{
	effect.setShader( "ClipShader" , isEnableFog ? "LightTech_Lit_Tex_Fog" : "LightTech_Lit_Tex" );
}

BasicCube::~BasicCube()
{
}

void BasicCube::createObjectMesh()
{
	XMFLOAT3 zero = XMFLOAT3( 0.0f , 0.0f , 0.0f );

	vertices =
	{
		{ XMFLOAT3( 1.0f,1.0f,1.0f ), zero , XMFLOAT2( 1.0f, 0.0f ) , XMFLOAT3( 1.0f,0.0f,0.0f ) },
		{ XMFLOAT3( -1.0f,1.0f,1.0f ), zero , XMFLOAT2( 0.0f, 0.0f ) , XMFLOAT3( 1.0f,0.0f,0.0f ) },
		{ XMFLOAT3( 1.0f,1.0f,-1.0f ), zero , XMFLOAT2( 1.0f, 1.0f ) , XMFLOAT3( 1.0f,0.0f,0.0f ) },
		{ XMFLOAT3( -1.0f,1.0f,-1.0f ), zero , XMFLOAT2( 0.0f, 1.0f ) , XMFLOAT3( 1.0f,0.0f,0.0f ) },//up

		{ XMFLOAT3( 1.0f,1.0f,1.0f ), zero , XMFLOAT2( 1.0f, 0.0f ) , XMFLOAT3( 1.0f,0.0f,0.0f ) },
		{ XMFLOAT3( -1.0f,1.0f,1.0f ), zero , XMFLOAT2( 0.0f, 0.0f ) , XMFLOAT3( 1.0f,0.0f,0.0f ) },
		{ XMFLOAT3( 1.0f,-1.0f,1.0f ), zero , XMFLOAT2( 1.0f, 1.0f ) , XMFLOAT3( 1.0f,0.0f,0.0f ) },
		{ XMFLOAT3( -1.0f,-1.0f,1.0f ), zero , XMFLOAT2( 0.0f, 1.0f ) , XMFLOAT3( 1.0f,0.0f,0.0f ) },//forward

		{ XMFLOAT3( 1.0f,1.0f,-1.0f ), zero , XMFLOAT2( 1.0f, 0.0f ) , XMFLOAT3( 1.0f,0.0f,0.0f ) },
		{ XMFLOAT3( -1.0f,1.0f,-1.0f ), zero , XMFLOAT2( 0.0f, 0.0f ) , XMFLOAT3( 1.0f,0.0f,0.0f ) },
		{ XMFLOAT3( 1.0f,-1.0f,-1.0f ), zero , XMFLOAT2( 1.0f, 1.0f ) , XMFLOAT3( 1.0f,0.0f,0.0f ) },
		{ XMFLOAT3( -1.0f,-1.0f,-1.0f ), zero , XMFLOAT2( 0.0f, 1.0f ) , XMFLOAT3( 1.0f,0.0f,0.0f ) },//back

		{ XMFLOAT3( -1.0f,1.0f,1.0f ), zero , XMFLOAT2( 1.0f, 0.0f ) , XMFLOAT3( 0.0f,0.0f,1.0f ) },
		{ XMFLOAT3( -1.0f,1.0f,-1.0f ), zero , XMFLOAT2( 0.0f, 0.0f ) , XMFLOAT3( 0.0f,0.0f,1.0f ) },
		{ XMFLOAT3( -1.0f,-1.0f,1.0f ), zero , XMFLOAT2( 1.0f, 1.0f ) , XMFLOAT3( 0.0f,0.0f,1.0f ) },
		{ XMFLOAT3( -1.0f,-1.0f,-1.0f ), zero , XMFLOAT2( 0.0f, 1.0f ) , XMFLOAT3( 0.0f,0.0f,1.0f ) },//left

		{ XMFLOAT3( 1.0f,1.0f,1.0f ), zero , XMFLOAT2( 1.0f, 0.0f ) , XMFLOAT3( 0.0f,0.0f,1.0f ) },
		{ XMFLOAT3( 1.0f,1.0f,-1.0f ), zero , XMFLOAT2( 0.0f, 0.0f ) , XMFLOAT3( 0.0f,0.0f,1.0f ) },
		{ XMFLOAT3( 1.0f,-1.0f,1.0f ), zero , XMFLOAT2( 1.0f, 1.0f ) , XMFLOAT3( 0.0f,0.0f,1.0f ) },
		{ XMFLOAT3( 1.0f,-1.0f,-1.0f ), zero , XMFLOAT2( 0.0f, 1.0f ) , XMFLOAT3( 0.0f,0.0f,1.0f ) },//right

		{ XMFLOAT3( 1.0f,-1.0f,1.0f ), zero , XMFLOAT2( 1.0f, 0.0f ) , XMFLOAT3( 1.0f,0.0f,0.0f ) },
		{ XMFLOAT3( -1.0f,-1.0f,1.0f ), zero , XMFLOAT2( 0.0f, 0.0f ) , XMFLOAT3( 1.0f,0.0f,0.0f ) },
		{ XMFLOAT3( 1.0f,-1.0f,-1.0f ), zero , XMFLOAT2( 1.0f, 1.0f ) , XMFLOAT3( 1.0f,0.0f,0.0f ) },
		{ XMFLOAT3( -1.0f,-1.0f,-1.0f ), zero , XMFLOAT2( 0.0f, 1.0f ) , XMFLOAT3( 1.0f,0.0f,0.0f ) }//down
	};

	indices =
	{
		0, 2, 3,
		0, 3, 1, // up

		4, 7, 6,
		4, 5, 7, // forward

		8, 11, 9,
		8, 10, 11, // back

		12, 13, 14,
		13, 15, 14, // left

		16, 18, 19,
		16, 19, 17, // right

		20, 23, 22,
		20, 21, 23  // down
	};

	computeNormal();
	computeBoundingBox();
}

void BasicCube::UpdateDirectionalLight( const DirectionalLight *dirLight , int lightNum )
{
	if ( lightNum < 0 )return;
	
	efDirLight->SetRawValue( dirLight , 0 , sizeof( DirectionalLight ) );
}

void BasicCube::createRenderState( ID3D11Device *device )
{
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory( &rsDesc , sizeof( D3D11_RASTERIZER_DESC ) );
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;

	device->CreateRasterizerState( &rsDesc , &rasterState );
}

void BasicCube::createObjectTexture( ID3D11Device *device )
{
	CreateDDSTextureFromFile( device , L"Textures/WireFence.dds" , &texture , &textureView );
}