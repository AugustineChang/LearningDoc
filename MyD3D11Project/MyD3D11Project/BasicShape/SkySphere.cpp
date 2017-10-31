#include "SkySphere.h"
#include "../Utilities/CommonHeader.h"
#include "../DirectXApp/Camera.h"
#include "DDSTextureLoader.h"
using namespace DirectX;


SkySphere::SkySphere()
{
	canPickup = false;
	effect.setShader( "SkySphereShader" , "CubemapTech_Tex" );
}


SkySphere::~SkySphere()
{
}

void SkySphere::createEffect( ID3D11Device *device )
{
	effect.createEffectAtBuildtime( device );

	efWVP = effect.getEffect()->GetVariableByName( "gWVP" )->AsMatrix();
	efTexture = effect.getEffect()->GetVariableByName( "cubeMap" )->AsShaderResource();
}


void SkySphere::createRenderState( ID3D11Device *device )
{
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory( &rsDesc , sizeof( D3D11_RASTERIZER_DESC ) );
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;

	device->CreateRasterizerState( &rsDesc , &rasterState );
}

void SkySphere::createObjectTexture( ID3D11Device *device )
{
	CreateDDSTextureFromFile( device , L"Textures/cubemap.dds" , &texture , &textureView );
}

void SkySphere::UpdateObjectEffect( const Camera *camera )
{
	buildWorldMatrix();

	XMMATRIX &tempW = XMLoadFloat4x4( &obj2World );
	XMMATRIX &tempV = camera->getViewMatrix();
	XMMATRIX &tempP = camera->getProjectMatrix();
	XMMATRIX tempWVP = tempW * tempV * tempP;

	efWVP->SetMatrix( reinterpret_cast<float*>( &tempWVP ) );
	efTexture->SetResource( textureView );
}
