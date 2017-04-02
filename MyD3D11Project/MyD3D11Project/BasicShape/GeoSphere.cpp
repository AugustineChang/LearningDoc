#include "GeoSphere.h"
#include "../Utilities/CommonHeader.h"
using namespace DirectX;


GeoSphere::GeoSphere() : radius( 2.0f ) , tesselTimes( 2 )
{
	material.specular.w = 1.0f;
}


GeoSphere::~GeoSphere()
{
}

void GeoSphere::createObjectMesh()
{
	XMFLOAT3 zero = XMFLOAT3( 0.0f , 0.0f , 0.0f );

	const float X = 0.525731f;
	const float Z = 0.850651f;
	vertices =
	{
		{ XMFLOAT3( -X, 0.0f, Z ), zero },
		{ XMFLOAT3( X, 0.0f, Z ), zero },
		{ XMFLOAT3( -X, 0.0f, -Z ), zero },
		{ XMFLOAT3( X, 0.0f, -Z ), zero },
		{ XMFLOAT3( 0.0f, Z, X ), zero },
		{ XMFLOAT3( 0.0f, Z, -X ), zero },
		{ XMFLOAT3( 0.0f, -Z, X ), zero },
		{ XMFLOAT3( 0.0f, -Z, -X ), zero },
		{ XMFLOAT3( Z, X, 0.0f ), zero },
		{ XMFLOAT3( -Z, X, 0.0f ), zero },
		{ XMFLOAT3( Z, -X, 0.0f ), zero },
		{ XMFLOAT3( -Z, -X, 0.0f ), zero }
	};
	indices =
	{
		1,4,0, 4,9,0, 4,5,9, 8,5,4, 1,8,4,
		1,10,8, 10,3,8, 8,3,5, 3,2,5, 3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9, 11,2,7
	};

	for ( UINT i = 0; i < tesselTimes; ++i )
	{
		doTessllation();
	}

	UINT len = vertices.size();
	for ( UINT i = 0; i < len; ++i )
	{
		XMVECTOR temp = XMLoadFloat3( &vertices[i].Pos );
		if ( i >= 12 )
		{
			temp = XMVector3Normalize( temp );
		}
		temp = temp * radius;

		XMStoreFloat3( &vertices[i].Pos , temp );
	}

	computeNormal();
}

void GeoSphere::doTessllation()
{
	// Ï¸·Ö·½·¨
	// 	     v1
	//       / \
	//      /   \
	//     /     \
	//    m0------m1
	//   /  \    / \
	//  /    \  /   \
	// /      \/     \
	//v0------m2------v2

	XMFLOAT3 zero = XMFLOAT3( 0.0f , 0.0f , 0.0f );

	UINT triangleNum = indices.size() / 3;
	for ( UINT i = 0; i < triangleNum; ++i )
	{
		//origin vertices
		UINT v0Index = indices[i * 3];
		UINT v1Index = indices[i * 3 + 1];
		UINT v2Index = indices[i * 3 + 2];
		CustomVertex v0 = vertices[v0Index];
		CustomVertex v1 = vertices[v1Index];
		CustomVertex v2 = vertices[v2Index];
		v0.Normal = zero;
		v1.Normal = zero;
		v2.Normal = zero;

		//add vertices
		UINT m0Index = vertices.size();
		UINT m1Index = m0Index + 1;
		UINT m2Index = m0Index + 2;
		vertices.push_back( { float3Mid( v0.Pos , v1.Pos ),zero } );//m0
		vertices.push_back( { float3Mid( v1.Pos , v2.Pos ),zero } );//m1
		vertices.push_back( { float3Mid( v0.Pos , v2.Pos ),zero } );//m2

		//add indices
		indices.push_back( m0Index );
		indices.push_back( v1Index );
		indices.push_back( m1Index );// m0 v1 m1

		indices.push_back( m2Index );
		indices.push_back( m0Index );
		indices.push_back( m1Index );// m2 m0 m1

		indices.push_back( m2Index );
		indices.push_back( m1Index );
		indices.push_back( v2Index );// m2 m1 v2

		//change old indices
		// v0 v1 v2 --> v0 m0 m2
		indices[i * 3 + 1] = m0Index;
		indices[i * 3 + 2] = m2Index;
	}
}

inline DirectX::XMFLOAT3 GeoSphere::float3Mid( const XMFLOAT3 &a , const XMFLOAT3 &b )
{
	return XMFLOAT3( ( a.x + b.x )*0.5f , ( a.y + b.y )*0.5f , ( a.z + b.z )*0.5f );
}

inline DirectX::XMFLOAT4 GeoSphere::float4Mid( const XMFLOAT4 &a , const XMFLOAT4 &b )
{
	return XMFLOAT4( ( a.x + b.x )*0.5f , ( a.y + b.y )*0.5f , ( a.z + b.z )*0.5f , ( a.w + b.w )*0.5f );
}
