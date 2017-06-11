#pragma once
#include <DirectXMath.h>

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

	void UpdatePosition( float raduis );
	void UpdatePosition2( float forwardSpeed , float rightSpeed );
	void UpdateRotation( float deltaX , float deltaY );

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
};

