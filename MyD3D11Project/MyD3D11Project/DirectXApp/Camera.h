#pragma once
#include <DirectXMath.h>
#include <DirectXCollision.h>

class Camera
{
public:
	Camera();
	~Camera();

	void buildViewMatrix();
	void buildProjectMatrix();
	void buildProjectMatrix( int screenWidth , int screenHeight );
	DirectX::XMVECTOR TransformDirection( DirectX::FXMVECTOR dir ) const;

	// Set frustum.
	void SetFrustum( float fovY , float aspect , float zn , float zf );

	DirectX::XMMATRIX getViewMatrix() const;
	DirectX::XMMATRIX getProjectMatrix() const;
	const DirectX::BoundingFrustum &getBoundingFrustum() const;

	void MoveCamera_Orbit( float raduis );
	void MoveCamera_Walk( float forwardSpeed , float rightSpeed );
	void RotateCamera( float deltaX , float deltaY );

	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT3 Rotation;

private:
	
	float fovAngle;
	float nearPlane;
	float farPlane;
	float aspectRatio;

	DirectX::XMFLOAT4X4 rotationMat;
	DirectX::XMFLOAT4X4 world2View;
	DirectX::XMFLOAT4X4 view2Proj;
	DirectX::BoundingFrustum frustum;
};

