#include "Cylinder.h"
#include "DDSTextureLoader.h"
#include "../Utilities/CommonHeader.h"
using namespace DirectX;


Cylinder::Cylinder() : topRadius( 0.8f ) , bottomRadius( 1.2f ) , height( 2.0f ) ,
	stackCount( 10 ) , sliceCount( 36 )
{
	effect.setShader( "LitShader" , isEnableFog ? "LightTech_Lit_Tex_Norm_Fog" : "LightTech_Lit_Tex_Norm" );
}


Cylinder::~Cylinder()
{
}


void Cylinder::UpdateObjectEffect( const Camera *camera )
{
	BasicShape::UpdateObjectEffect( camera );

	efNormalTex->SetResource( normalTexView );
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
	float deltaAngle = 2.0f * SimpleMath::PI / sliceCount;
	for ( UINT i = 0; i < sliceCount; ++i )
	{
		BaseVertex top;
		top.Pos = XMFLOAT3( 0.0f , halfHeight , 0.0f );
		top.Normal = XMFLOAT3( 0.0f , 0.0f , 0.0f );
		top.TexCoord = XMFLOAT2( ( ( i + 0.5f ) * deltaAngle ) / ( SimpleMath::PI * 2 ) , 0.0f );
		top.TangentU = XMFLOAT3( 1.0f , 0.0f , 0.0f );
		vertices.push_back( top );
	}

	//bottom vertex
	for ( UINT i = 0; i < sliceCount; ++i )
	{
		BaseVertex bottom;
		bottom.Pos = XMFLOAT3( 0.0f , -halfHeight , 0.0f );
		bottom.Normal = XMFLOAT3( 0.0f , 0.0f , 0.0f );
		bottom.TexCoord = XMFLOAT2( ( ( i + 0.5f ) * deltaAngle ) / ( SimpleMath::PI * 2 ) , 1.0f );
		bottom.TangentU = XMFLOAT3( 1.0f , 0.0f , 0.0f );
		vertices.push_back( bottom );
	}

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
	for ( UINT vert = 0; vert < sliceCount; ++vert )
	{
		UINT curIndex = ( stackCount - 1 )*( sliceCount + 1 ) + vert;
		UINT nextIndex = curIndex + 1;
		UINT topIndex = vertices.size() - sliceCount * 2 + vert;

		indices.push_back( curIndex );
		indices.push_back( topIndex );
		indices.push_back( nextIndex );
	}

	//bottom index
	for ( UINT vert = 0; vert < sliceCount; ++vert )
	{
		UINT curIndex = vert;
		UINT nextIndex = curIndex + 1;
		UINT bottomIndex = vertices.size() - sliceCount + vert;

		indices.push_back( curIndex );
		indices.push_back( nextIndex );
		indices.push_back( bottomIndex );
	}

	computeNormal();
	computeBoundingBox();
}


void Cylinder::createEffect( ID3D11Device *device )
{
	BasicShape::createEffect( device );

	efNormalTex = effect.getEffect()->GetVariableByName( "normalTex" )->AsShaderResource();
}

void Cylinder::createObjectTexture( struct ID3D11Device *device )
{
	CreateDDSTextureFromFile( device , L"Textures/stones.dds" , &texture , &textureView );
	CreateDDSTextureFromFile( device , L"Textures/stones_nmap.dds" , &normalTex , &normalTexView );
}

void Cylinder::generateCicle( float radius , float yPos )
{
	float deltaAngle = 2.0f * SimpleMath::PI / sliceCount;
	float halfHeight = height * 0.5f;

	for ( UINT i = 0; i < sliceCount; ++i )
	{
		BaseVertex one;
		float cos = cosf( deltaAngle*i );
		float sin = sinf( deltaAngle*i );

		one.Pos = XMFLOAT3( radius * cos , yPos , radius * sin );
		one.Normal = XMFLOAT3( 0.0f , 0.0f , 0.0f );
		one.TexCoord.x = ( i * deltaAngle ) / ( SimpleMath::PI * 2 );
		one.TexCoord.y = 1 - ( ( yPos + halfHeight + topRadius ) / ( topRadius + height + bottomRadius ) );
		one.TangentU = XMFLOAT3( -sin , 0.0f , cos );
		vertices.push_back( one );
	}

	//duplicate first
	UINT size = vertices.size();
	vertices.push_back( vertices[size - sliceCount] );
	vertices[size].TexCoord.x = 1.0f;
}
