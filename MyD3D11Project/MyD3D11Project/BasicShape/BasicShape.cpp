#include "BasicShape.h"
#include "../Utilities/CommonHeader.h"
using namespace DirectX;

BasicShape::BasicShape()
{
	XMMATRIX identityMaxtrix = DirectX::XMMatrixIdentity();
	XMStoreFloat4x4( &obj2World , identityMaxtrix );

	XMVECTOR objPos = XMVectorSet( 0.0f , 0.0f , 0.0f , 1.0f );
	XMStoreFloat4( &Position , objPos );

	XMVECTOR objRot = XMVectorSet( 0.0f , 0.0f , 0.0f , 0.0f );
	XMStoreFloat3( &Rotation , objRot );

	XMVECTOR objScale = XMVectorSet( 1.0f , 1.0f , 1.0f , 0.0f );
	XMStoreFloat3( &Scale , objScale );

	material.ambient = XMFLOAT4( 1.0f , 1.0f , 1.0f , 1.0f );
	material.diffuse = XMFLOAT4( 1.0f , 1.0f , 1.0f , 1.0f );
	material.specular = XMFLOAT4( 1.0f , 1.0f , 1.0f , 5.0f );
}

BasicShape::~BasicShape()
{
	ReleaseCOM( textureView );
	ReleaseCOM( texture );
}

void BasicShape::buildWorldMatrix( )
{
	FXMVECTOR scale = XMLoadFloat3( &Scale );
	FXMVECTOR rotation = XMLoadFloat3( &Rotation );
	FXMVECTOR position = XMLoadFloat4( &Position );

	XMMATRIX scaleMat = DirectX::XMMatrixScalingFromVector( scale );
	XMMATRIX rotationMat = DirectX::XMMatrixRotationRollPitchYawFromVector( rotation );
	XMMATRIX transformMat = DirectX::XMMatrixTranslationFromVector( position );

	XMMATRIX temp = scaleMat * rotationMat * transformMat;
	XMStoreFloat4x4( &obj2World , temp );
}

void BasicShape::InitShape( struct ID3D11Device *device )
{
	createObjectMesh();
	createObjectTexture( device );
	createBlendState( device );
}

const std::vector<CustomVertex>& BasicShape::getVertices() const
{
	return vertices;
}

const std::vector<unsigned int>& BasicShape::getIndices() const
{
	return indices;
}

const CustomMaterial& BasicShape::getMaterial() const
{
	return material;
}

ID3D11ShaderResourceView* BasicShape::getTexture() const
{
	return textureView;
}

ID3D11ShaderResourceView* BasicShape::getAlphaTexture() const
{
	return alphaTextureView;
}

ID3D11BlendState* BasicShape::getBlendState() const
{
	return blendState;
}

DirectX::XMMATRIX BasicShape::getWorldMatrix() const
{
	return XMLoadFloat4x4( &obj2World );
}

void BasicShape::computeNormal()
{
	unsigned int indexNum = indices.size();
	unsigned int triangleNum = indexNum / 3;
	for ( unsigned int i = 0; i < triangleNum; ++i )
	{
		unsigned int i0 = indices[i * 3];
		unsigned int i1 = indices[i * 3 + 1];
		unsigned int i2 = indices[i * 3 + 2];

		XMVECTOR v0 = XMLoadFloat3( &vertices[i0].Pos );
		XMVECTOR v1 = XMLoadFloat3( &vertices[i1].Pos );
		XMVECTOR v2 = XMLoadFloat3( &vertices[i2].Pos );

		XMVECTOR u = v1 - v0;
		XMVECTOR v = v2 - v0;
		XMVECTOR normal = XMVector3Cross( u , v );

		XMVECTOR originNor = XMLoadFloat3( &vertices[i0].Normal );
		XMStoreFloat3( &vertices[i0].Normal , originNor + normal );
		originNor = XMLoadFloat3( &vertices[i1].Normal );
		XMStoreFloat3( &vertices[i1].Normal , originNor + normal );
		originNor = XMLoadFloat3( &vertices[i2].Normal );
		XMStoreFloat3( &vertices[i2].Normal , originNor + normal );
	}

	unsigned int vertexNum = vertices.size();
	for ( unsigned int i = 0; i < vertexNum; ++i )
	{
		XMVECTOR normal = XMLoadFloat3( &vertices[i].Normal );
		normal = XMVector3Normalize( normal );
		XMStoreFloat3( &vertices[i].Normal , normal );
	}
		
}
