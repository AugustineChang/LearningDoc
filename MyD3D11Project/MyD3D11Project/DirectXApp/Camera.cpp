#include "Camera.h"
#include "../Utilities/CommonHeader.h"
using namespace DirectX;

Camera::Camera() : fovAngle( SimpleMath::PI / 3.0f ) ,
nearPlane( 1.0f ) , farPlane( 300.0f )
{
	XMMATRIX identityMaxtrix = DirectX::XMMatrixIdentity();
	XMStoreFloat4x4( &world2View , identityMaxtrix );
	XMStoreFloat4x4( &view2Proj , identityMaxtrix );

	XMVECTOR camPos = XMVectorSet( 0.0f , 0.0f , 0.0f , 1.0f );
	XMStoreFloat4( &Position , camPos );

	XMVECTOR camRot = XMVectorSet( 0.0f , 0.0f , 0.0f , 0.0f );
	XMStoreFloat3( &Rotation , camRot );
}


Camera::~Camera()
{
}

void Camera::buildViewMatrix()
{
	XMMATRIX rotMatrix =
		DirectX::XMMatrixRotationRollPitchYawFromVector( XMLoadFloat3( &Rotation ) );
	XMStoreFloat4x4( &rotationMat , rotMatrix );

	XMVECTOR position = XMLoadFloat4( &Position );
	XMVECTOR forward = XMVectorSet( 0 , 0 , 1 , 0 );
	forward = DirectX::XMVector4Transform( forward , rotMatrix );
	XMVECTOR up = XMVectorSet( 0 , 1 , 0 , 0 );

	XMMATRIX temp = DirectX::XMMatrixLookToLH( position , forward , up );
	XMStoreFloat4x4( &world2View , temp );
}

void Camera::buildProjectMatrix( int screenWidth , int screenHeight )
{
	float aspectRatio = (float) screenWidth / screenHeight;
	XMMATRIX temp = DirectX::XMMatrixPerspectiveFovLH( fovAngle , aspectRatio , nearPlane , farPlane );
	XMStoreFloat4x4( &view2Proj , temp );
}

DirectX::XMMATRIX Camera::getViewMatrix() const
{
	return XMLoadFloat4x4( &world2View );
}

DirectX::XMMATRIX Camera::getProjectMatrix() const
{
	return XMLoadFloat4x4( &view2Proj );
}

void Camera::UpdatePosition( float radius )
{
	Position.x = radius * cosf( Rotation.x ) * cosf( -Rotation.y - SimpleMath::PI / 2 );
	Position.z = radius * cosf( Rotation.x ) * sinf( -Rotation.y - SimpleMath::PI / 2 );
	Position.y = radius * sinf( Rotation.x );
}

void Camera::UpdatePosition2( float speed )
{
	XMMATRIX rotMatrix = XMLoadFloat4x4( &rotationMat );

	XMVECTOR forward = XMVectorSet( 0 , 0 , 1 , 0 );
	forward = DirectX::XMVector4Transform( forward , rotMatrix );

	XMVECTOR curPosition = XMLoadFloat4( &Position );
	XMStoreFloat4( &Position , curPosition + forward * speed );
}

void Camera::UpdateRotation( float deltaX , float deltaY )
{
	Rotation.y += deltaX;
	Rotation.x = SimpleMath::Clamp<float>( Rotation.x + deltaY , -SimpleMath::PI / 2 + 0.01f , SimpleMath::PI / 2 - 0.01f );
}
