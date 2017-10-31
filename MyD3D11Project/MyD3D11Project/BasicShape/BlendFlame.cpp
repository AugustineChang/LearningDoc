#include "BlendFlame.h"
#include "../Utilities/CommonHeader.h"
#include "../DirectXApp/Camera.h"
#include "../DirectXApp/Lights.h"
#include "WICTextureLoader.h"
#include <sstream>
using namespace DirectX;


BlendFlame::BlendFlame() : timer( 0.0f ) , curTexture( 0 )
{
	isEnableFog = false;
	effect.setShader( "BlendShader" , "LightTech_Lit_Tex" );
}

BlendFlame::~BlendFlame()
{
	for ( auto texView : textureViews ) ReleaseCOM( texView );
	for ( auto tex : textures ) ReleaseCOM( tex );
}

void BlendFlame::UpdateObject( float DeltaTime , ID3D11DeviceContext *immediateContext )
{
	if ( !isPassFrustumTest ) return;

	float interval = 0.0333f;
	timer += DeltaTime;
	if ( timer > interval )
	{
		timer -= interval;
		curTexture = ( curTexture + 1 ) % 120;
		texture = textures[curTexture];
		textureView = textureViews[curTexture];
	}
}

void BlendFlame::createObjectMesh()
{
	XMFLOAT3 zero = XMFLOAT3( 0.0f , 0.0f , 0.0f );

	vertices =
	{
		{ XMFLOAT3( 1.0f,1.0f,0.0f ), zero , XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( -1.0f,1.0f,0.0f ), zero , XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( 1.0f,-1.0f,0.0f ), zero , XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f,-1.0f,0.0f ), zero , XMFLOAT2( 0.0f, 1.0f ) }
	};

	indices =
	{
		0, 2, 3,
		0, 3, 1
	};

	computeNormal();
	computeBoundingBox();
}

void BlendFlame::createBlendState( struct ID3D11Device *device )
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

void BlendFlame::createObjectTexture( ID3D11Device *device )
{
	textures.reserve( 120 );
	textureViews.reserve( 120 );

	for ( int i = 1; i <= 120; ++i )
	{
		std::wstringstream wss;
		wss << L"Textures/FireAnim/Fire";
		if ( i < 10 ) wss << L"00" << i;
		else if ( i < 100 ) wss << L"0" << i;
		else wss << i;
		wss << L".bmp";

		ID3D11Resource *tex;
		ID3D11ShaderResourceView *texView;
		CreateWICTextureFromFile( device , wss.str().c_str() , &tex , &texView );
		textures.push_back( tex );
		textureViews.push_back( texView );
	}

	texture = textures[curTexture];
	textureView = textureViews[curTexture];
}

void BlendFlame::UpdateDirectionalLight( const DirectionalLight *dirLight , int lightNum )
{
	if ( lightNum < 0 )return;

	efDirLight->SetRawValue( dirLight , 0 , sizeof( DirectionalLight ) );
}
