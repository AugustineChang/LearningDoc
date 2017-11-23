#include "Sphere.h"
#include "DDSTextureLoader.h"
#include "../Utilities/CommonHeader.h"
#include "../DirectXApp/Camera.h"
using namespace DirectX;


Sphere::Sphere() : stackCount( 17 ) , sliceCount( 20 ) , radius( 2.0f )
{
	//material.specular = XMFLOAT4( 0.0f , 0.0f , 0.0f , 1.0f );
	material->specular.w = 3.0f;
	//effect.setShader( "LitShader" , isEnableFog ? "LightTech_Lit_Tex_Norm_Fog" : "LightTech_Lit_Tex_Norm" );
	effect.setShader( "LitShader_DM" , "LightTech_DisplacementMap" );
}


Sphere::~Sphere()
{
}


void Sphere::UpdateObjectEffect( const Camera *camera )
{
	BasicShape::UpdateObjectEffect( camera );

	efNormalTex->SetResource( normalTexView );

	XMMATRIX &tempV = camera->getViewMatrix();
	XMMATRIX &tempP = camera->getProjectMatrix();
	XMMATRIX tempVP = XMMatrixMultiply( tempV , tempP );

	efVP->SetMatrix( reinterpret_cast<float*>( &tempVP ) );
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
	float deltaAngle = 2.0f * SimpleMath::PI / sliceCount;
	for ( UINT i = 0; i < sliceCount; ++i )
	{
		BaseVertex top;
		top.Pos = XMFLOAT3( 0.0f , radius , 0.0f );
		top.Normal = XMFLOAT3( 0.0f , 0.0f , 0.0f );
		top.TexCoord = XMFLOAT2( ( ( i + 0.5f ) * deltaAngle ) / ( SimpleMath::PI * 2 ) , 0.0f );
		top.TangentU = XMFLOAT3( 1.0f , 0.0f , 0.0f );
		vertices.push_back( top );
	}

	//bottom vertex
	for ( UINT i = 0; i < sliceCount; ++i )
	{
		BaseVertex bottom;
		bottom.Pos = XMFLOAT3( 0.0f , -radius , 0.0f );
		bottom.Normal = XMFLOAT3( 0.0f , 0.0f , 0.0f );
		bottom.TexCoord = XMFLOAT2( ( ( i + 0.5f ) * deltaAngle ) / ( SimpleMath::PI * 2 ) , 1.0f );
		bottom.TangentU = XMFLOAT3( 1.0f , 0.0f , 0.0f );
		vertices.push_back( bottom );
	}

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
	for ( UINT vert = 0; vert < sliceCount; ++vert )
	{
		UINT curIndex = vert;
		UINT nextIndex = curIndex + 1;
		UINT topIndex = vertices.size() - sliceCount * 2 + vert;

		indices.push_back( curIndex );
		indices.push_back( topIndex );
		indices.push_back( nextIndex );
	}

	//bottom index
	for ( UINT vert = 0; vert < sliceCount; ++vert )
	{
		UINT curIndex = ( stackCount - 3 )*( sliceCount + 1 ) + vert;
		UINT nextIndex = curIndex + 1;
		UINT bottomIndex = vertices.size() - sliceCount + vert;

		indices.push_back( curIndex );
		indices.push_back( nextIndex );
		indices.push_back( bottomIndex );
	}

	computeNormal();
	computeBoundingBox();
}


void Sphere::createEffect( ID3D11Device *device )
{
	BasicShape::createEffect( device );

	efNormalTex = effect.getEffect()->GetVariableByName( "normalTex" )->AsShaderResource();
	efVP = effect.getEffect()->GetVariableByName( "gVP" )->AsMatrix();
}

void Sphere::createObjectTexture( struct ID3D11Device *device )
{
	CreateDDSTextureFromFile( device , L"Textures/stones.dds" , &texture , &textureView );
	CreateDDSTextureFromFile( device , L"Textures/stones_nmap.dds" , &normalTex , &normalTexView );
}

void Sphere::createRenderState( ID3D11Device *device )
{
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory( &rsDesc , sizeof( D3D11_RASTERIZER_DESC ) );
	rsDesc.FillMode = D3D11_FILL_WIREFRAME;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;

	device->CreateRasterizerState( &rsDesc , &rasterState );
}


void Sphere::RenderObject( ID3D11DeviceContext *immediateContext )
{
	immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST );
	immediateContext->IASetInputLayout( inputLayout );
	immediateContext->RSSetState( rasterState );

	float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	immediateContext->OMSetBlendState( blendState , blendFactors , 0xffffffff );

	ID3DX11EffectTechnique *technique = effect.getEffectTech();
	D3DX11_TECHNIQUE_DESC techDesc;
	technique->GetDesc( &techDesc );
	for ( UINT i = 0; i < techDesc.Passes; ++i )
	{
		technique->GetPassByIndex( i )->Apply( 0 , immediateContext );

		immediateContext->DrawIndexed( indexSize , indexStart , indexBase );
	}
}

void Sphere::generateCicle( float vAngle )
{
	float deltaHAngle = 2.0f * SimpleMath::PI / sliceCount;
	XMFLOAT3 zero = XMFLOAT3( 0.0f , 0.0f , 0.0f );

	for ( UINT i = 0; i < sliceCount; ++i )
	{
		BaseVertex one;

		float hAngle = deltaHAngle*i;

		one.Pos = XMFLOAT3( radius * sinf( vAngle ) * cosf( hAngle ) ,
						   radius * cosf( vAngle ) ,
						   radius * sinf( vAngle ) * sinf( hAngle ) );
		one.Normal = zero;
		one.TexCoord = SimpleMath::Div( XMFLOAT2( hAngle , vAngle ) , XMFLOAT2( 2.0f*SimpleMath::PI , SimpleMath::PI ) );
		one.TangentU.x = -radius*sinf( vAngle )*sinf( hAngle );
		one.TangentU.y = 0.0f;
		one.TangentU.z = +radius*sinf( vAngle )*cosf( hAngle );

		XMVECTOR T = XMLoadFloat3( &one.TangentU );
		XMStoreFloat3( &one.TangentU , XMVector3Normalize( T ) );

		vertices.push_back( one );
	}

	//duplicate first
	UINT size = vertices.size();
	vertices.push_back( vertices[size - sliceCount] );
	vertices[size].TexCoord.x = 1.0f;
}