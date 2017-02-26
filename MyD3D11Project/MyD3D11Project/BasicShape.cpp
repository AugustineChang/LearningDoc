#include "BasicShape.h"
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
}

BasicShape::~BasicShape()
{

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

DirectX::XMMATRIX BasicShape::getWorldMatrix() const
{
	return XMLoadFloat4x4( &obj2World );
}
