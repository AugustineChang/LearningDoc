#pragma once
#include "BaseShape.h"


class Camera : public BaseShape
{
public:

	Camera();
	Camera( float hfov , float aspect , float near , float far );
	Camera( const Camera& copy );
	~Camera();

	void operator=( const Camera& copy );

	void rotateCameraByLookAt( const Vector3& lookPoint , const Vector3& upDir = Vector3( 0.0f , 0.0f , 1.0f ) );

	Matrix getViewMatrix() const
	{
		return viewMatrix;
	}

	Matrix getClipMatrix() const
	{
		return clipMatrix;
	}

	float getFOV() const { return hFOV; }
	float getAspect() const { return aspect; }
	float getNearDist() const { return nearDist; }
	float getFarDist() const { return farDist; }

private:

	void updateClipMatrix();
	virtual void updateWorldMatrix() override;

	//给定量
	float hFOV;
	float aspect;
	float nearDist;
	float farDist;

	//计算量
	Matrix clipMatrix;
	Matrix viewMatrix;
};

