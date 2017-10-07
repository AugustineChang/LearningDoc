#include "Camera.h"
#include "../Utilities/CommonHeader.h"
#include <math.h>

using namespace DirectX;

Camera::Camera() : fovAngle( SimpleMath::PI / 3.0f ) ,
nearPlane( 0.1f ) , farPlane( 300.0f ) , aspectRatio( 16.0f / 9.0f )
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

void Camera::buildProjectMatrix( int screenW , int screenH )
{
	screenWidth = screenW;
	screenHeight = screenH;

	aspectRatio = (float) screenWidth / screenHeight;
	XMMATRIX temp = DirectX::XMMatrixPerspectiveFovLH( fovAngle , aspectRatio , nearPlane , farPlane );
	XMStoreFloat4x4( &view2Proj , temp );

	BoundingFrustum::CreateFromMatrix( frustum , temp );
}

void Camera::buildProjectMatrix()
{
	XMMATRIX temp = DirectX::XMMatrixPerspectiveFovLH( fovAngle , aspectRatio , nearPlane , farPlane );
	XMStoreFloat4x4( &view2Proj , temp );

	BoundingFrustum::CreateFromMatrix( frustum , temp );
}

DirectX::XMVECTOR Camera::TransformDirection( DirectX::FXMVECTOR dir ) const
{
	XMMATRIX rotMatrix = XMLoadFloat4x4( &rotationMat );
	return DirectX::XMVector4Transform( dir , rotMatrix );
}

void Camera::SetFrustum( float fovY , float aspect , float zn , float zf )
{
	fovAngle = fovY;
	aspectRatio = aspect;

	nearPlane = zn;
	farPlane = zf;
}

DirectX::XMVECTOR Camera::getScreenClickPoint( float screenX , float screenY , float depth )
{
	float clickX_view = ( 2.0f * screenX / screenWidth - 1.0f ) / view2Proj( 0 , 0 );
	float clickY_view = ( -2.0f * screenY / screenHeight + 1.0f ) / view2Proj( 1 , 1 );

	XMVECTOR clickPoint_view = XMVectorSet( clickX_view * depth , clickY_view * depth , depth , 1.0f );

	XMMATRIX viewMat = XMLoadFloat4x4( &world2View );
	XMMATRIX view2world = XMMatrixInverse( &XMMatrixDeterminant( viewMat ) , viewMat );
	return XMVector4Transform( clickPoint_view , view2world );
}

DirectX::XMVECTOR Camera::getScreenRay( float screenX , float screenY )
{
	XMVECTOR position = XMLoadFloat4( &Position );
	return getScreenClickPoint( screenX , screenY , 1.0f ) - position;
}

DirectX::XMMATRIX Camera::getViewMatrix() const
{
	return XMLoadFloat4x4( &world2View );
}

DirectX::XMMATRIX Camera::getProjectMatrix() const
{
	return XMLoadFloat4x4( &view2Proj );
}

const DirectX::BoundingFrustum &Camera::getBoundingFrustum() const
{
	return frustum;
}

void Camera::MoveCamera_Orbit( float radius )
{
	Position.x = radius * cosf( Rotation.x ) * cosf( -Rotation.y - SimpleMath::PI / 2 );
	Position.z = radius * cosf( Rotation.x ) * sinf( -Rotation.y - SimpleMath::PI / 2 );
	Position.y = radius * sinf( Rotation.x );
}

void Camera::MoveCamera_Walk( float forwardSpeed , float rightSpeed )
{
	if ( forwardSpeed == 0.0f && rightSpeed == 0.0f ) return;

	XMMATRIX rotMatrix = XMLoadFloat4x4( &rotationMat );

	XMVECTOR moveDir = XMVectorSet( rightSpeed , 0 , forwardSpeed , 0 );
	moveDir = DirectX::XMVector4Transform( moveDir , rotMatrix );

	XMVECTOR curPosition = XMLoadFloat4( &Position );
	XMStoreFloat4( &Position , curPosition + moveDir );
}

void Camera::RotateCamera( float deltaX , float deltaY )
{
	Rotation.y += deltaX;
	Rotation.x = SimpleMath::Clamp<float>( Rotation.x + deltaY , -SimpleMath::PI / 2 + 0.01f , SimpleMath::PI / 2 - 0.01f );
}
