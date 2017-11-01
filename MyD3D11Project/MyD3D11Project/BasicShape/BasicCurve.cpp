#include "BasicCurve.h"
#include "../Utilities/CommonHeader.h"
#include "../DirectXApp/Lights.h"
#include "DDSTextureLoader.h"
#include <sstream>
using namespace DirectX;


BasicCurve::BasicCurve() : BasicShape() , sliceCount( 12 ) , lineWidth( 0.5f )
{
	controlPoints =
	{
		XMFLOAT3( 0.0f , 0.0f , 0.0f ),
		XMFLOAT3( 2.5f , 0.0f , 2.0f ),
		XMFLOAT3( 4.0f , 0.0f , -2.0f ),
		XMFLOAT3( 5.0f , 0.0f , 0.0f )
	};
}

BasicCurve::~BasicCurve()
{
}

void BasicCurve::createObjectMesh()
{
	XMFLOAT3 zero = XMFLOAT3( 0.0f , 0.0f , 0.0f );

	std::vector<XMVECTOR> tempList;
	int len = controlPoints.size();
	for ( int i = 0; i < len; ++i )
	{
		tempList.push_back( XMLoadFloat3( &controlPoints[i] ) );
	}

	XMFLOAT3 tangent;
	XMStoreFloat3( &tangent , tempList[len - 1] - tempList[0] );

	for ( int i = 0; i < sliceCount; ++i )
	{
		float alpha = (float) i / ( sliceCount - 1 );
		XMVECTOR result;
		assert( getPointOnCurve( alpha , tempList , result ) );
		XMFLOAT3 location;
		XMStoreFloat3( &location , result );

		BaseVertex vertex = { location , zero , XMFLOAT2( alpha , 0.1f ) , tangent };
		BaseVertex vertex2 = { XMFLOAT3( location.x , location.y , location.z + lineWidth ) , zero , XMFLOAT2( alpha , 0.0f ) , tangent };

		vertices.push_back( vertex );
		vertices.push_back( vertex2 );
	}

	for ( int i = 0; i < sliceCount - 1; ++i )
	{
		UINT curVertex = i * 2;
		UINT curUpVertex = i * 2 + 1;
		UINT nextVertex = ( i + 1 ) * 2;
		UINT nextUpVertex = ( i + 1 ) * 2 + 1;

		indices.push_back( curVertex );
		indices.push_back( curUpVertex );
		indices.push_back( nextUpVertex );

		indices.push_back( curVertex );
		indices.push_back( nextUpVertex );
		indices.push_back( nextVertex );
	}

	computeNormal();
	computeBoundingBox();
}

void BasicCurve::createRenderState( ID3D11Device *device )
{
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory( &rsDesc , sizeof( D3D11_RASTERIZER_DESC ) );
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;

	device->CreateRasterizerState( &rsDesc , &rasterState );
}

void BasicCurve::createObjectTexture( ID3D11Device *device )
{
	CreateDDSTextureFromFile( device , L"Textures/ice.dds" , &texture , &textureView );
}

bool BasicCurve::getPointOnCurve( float alpha , const std::vector<XMVECTOR> &points , XMVECTOR &outData )
{
	int len = points.size();
	if ( len < 2 ) return false;
	else if ( len == 2 )
	{
		outData = DirectX::XMVectorLerp( points[0] , points[1] , alpha );
		return true;
	}
	else
	{
		std::vector<XMVECTOR> tempList;
		for ( int i = 0; i < len - 1; ++i )
		{
			XMVECTOR temp = DirectX::XMVectorLerp( points[i] , points[i + 1] , alpha );
			tempList.push_back( temp );
		}
		return getPointOnCurve( alpha , tempList , outData );
	}
}
