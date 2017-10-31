#include "RotateFlame.h"
#include "../Utilities/CommonHeader.h"
#include "../DirectXApp/Camera.h"
#include "../DirectXApp/Lights.h"
#include "DDSTextureLoader.h"
#include <sstream>
using namespace DirectX;


RotateFlame::RotateFlame()
{
	isEnableFog = false;
	effect.setShader( "RotateFlame" , "LightTech_Lit_Tex" );
}

RotateFlame::~RotateFlame()
{
}

void RotateFlame::UpdateObject( float DeltaTime , ID3D11DeviceContext *immediateContext )
{
	if ( !isPassFrustumTest ) return;

	rotAngle += DeltaTime * 0.1f;
	rotMatrix = XMMatrixTranslation( -0.5f , -0.5f , 0.0f ) *
	XMMatrixRotationZ( rotAngle ) * XMMatrixTranslation( 0.5f , 0.5f , 0.0f );
}

void RotateFlame::createObjectMesh()
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

void RotateFlame::createEffect( ID3D11Device *device )
{
	BasicShape::createEffect( device );
	efAlphaTexture = effect.getEffect()->GetVariableByName( "diffuseAlphaTex" )->AsShaderResource();
}

void RotateFlame::createObjectTexture( ID3D11Device *device )
{
	CreateDDSTextureFromFile( device , L"Textures/flare.dds" , &texture , &textureView );
	CreateDDSTextureFromFile( device , L"Textures/flarealpha.dds" , &alphaTexture , &alphaTextureView );
}

void RotateFlame::UpdateObjectEffect( const Camera *camera )
{
	buildWorldMatrix();

	XMMATRIX &tempW = XMLoadFloat4x4( &obj2World );
	XMMATRIX &tempV = camera->getViewMatrix();
	XMMATRIX &tempP = camera->getProjectMatrix();
	XMMATRIX tempWVP = tempW * tempV * tempP;

	XMMATRIX inverseW = XMMatrixInverse( &XMMatrixDeterminant( tempW ) , tempW );
	XMMATRIX inverseTransposeW = XMMatrixTranspose( inverseW );

	efWVP->SetMatrix( reinterpret_cast<float*>( &tempWVP ) );
	efWorld->SetMatrix( reinterpret_cast<const float*>( &tempW ) );
	efWorldNorm->SetMatrix( reinterpret_cast<const float*>( &inverseTransposeW ) );
	efTexTrans->SetMatrix( reinterpret_cast<const float*>( &rotMatrix ) );

	efMaterial->SetRawValue( &material , 0 , sizeof( CustomMaterial ) );
	efTexture->SetResource( textureView );
	efAlphaTexture->SetResource( alphaTextureView );
	
	efCameraPos->SetRawValue( &( camera->Position ) , 0 , sizeof( XMFLOAT3 ) );
}

void RotateFlame::UpdateDirectionalLight( const DirectionalLight *dirLight , int lightNum )
{
	if ( lightNum < 0 )return;

	efDirLight->SetRawValue( dirLight , 0 , sizeof( DirectionalLight ) );
}
