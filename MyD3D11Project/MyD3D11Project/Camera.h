#pragma once
#include <DirectXMath.h>

class Camera
{
public:
	Camera();
	~Camera();

	void buildViewMatrix();
	void buildProjectMatrix( int screenWidth , int screenHeight );

	DirectX::XMMATRIX getViewMatrix();
	DirectX::XMMATRIX getProjectMatrix();

	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT3 Rotation;

private:
	
	float fovAngle;
	float nearPlane;
	float farPlane;

	DirectX::XMFLOAT4X4 world2View;
	DirectX::XMFLOAT4X4 view2Proj;
};

