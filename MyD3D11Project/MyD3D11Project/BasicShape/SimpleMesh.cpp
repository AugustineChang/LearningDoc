#include "SimpleMesh.h"
#include "../Utilities/CommonHeader.h"
#include <fstream>
#include <string>


SimpleMesh::SimpleMesh() : filePath( "Models/car.txt" )
{
	createObjectMesh();
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
		fin >> vertices[i].Color.x >> vertices[i].Color.y >> vertices[i].Color.z;
		vertices[i].Color.w = 1.0f;
	}

	fin >> ignore >> ignore >> ignore;

	UINT indexCount = triCount * 3;
	indices.resize( indexCount );
	for ( UINT i = 0; i < indexCount; ++i )
	{
		fin >> indices[i];
	}

	fin.close();
}
