#include "SimpleMesh.h"
#include "../Utilities/CommonHeader.h"
#include "DDSTextureLoader.h"
#include <fstream>
#include <string>


SimpleMesh::SimpleMesh() : filePath( "Models/skull.txt" )
{
	effect.setShader( "LitReflectShader" , isEnableFog ? "LightTech_Lit_Tex_Fog" : "LightTech_Lit_Tex" );
}

SimpleMesh::~SimpleMesh()
{
}

void SimpleMesh::createObjectMesh()
{
	std::ifstream fin( filePath.c_str() );
	if ( !fin )
	{
		MessageBox( 0 , L"Model file not found!!!" , 0 , 0 );
		return;
	}

	UINT vertCount;
	UINT triCount;
	std::string ignore;

	fin >> ignore >> vertCount;
	fin >> ignore >> triCount;
	fin >> ignore >> ignore >> ignore >> ignore;
	
	vertices.resize( vertCount );
	for ( UINT i = 0; i < vertCount; ++i )
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	fin >> ignore >> ignore >> ignore;

	UINT indexCount = triCount * 3;
	indices.resize( indexCount );
	for ( UINT i = 0; i < indexCount; ++i )
	{
		fin >> indices[i];
	}

	fin.close();

	computeBoundingBox();
}

void SimpleMesh::createEffect( ID3D11Device *device )
{
	BasicShape::createEffect( device );

	efReflTexture = effect.getEffect()->GetVariableByName( "cubeMap" )->AsShaderResource();
}

void SimpleMesh::createObjectTexture( ID3D11Device *device )
{
	DirectX::CreateDDSTextureFromFile( device , L"Textures/water1.dds" , &texture , &textureView );
	DirectX::CreateDDSTextureFromFile( device , L"Textures/cubemap.dds" , &reflTexture , &reflTextureView );
}

void SimpleMesh::UpdateObjectEffect( const Camera *camera )
{
	BasicShape::UpdateObjectEffect( camera );

	efReflTexture->SetResource( reflTextureView );
}
