#include "Camera.h"
#include "SimpleMath.h"
#include <math.h>

Camera::Camera() : BaseShape() ,  hFOV( 90.0f ) , aspect( 2.0f ) , nearDist(1.0f) , farDist( 500.0f )
{
	updateClipMatrix();
}

Camera::Camera( float hfov , float aspect , float near , float far )
	: BaseShape() , hFOV( hfov ) , aspect( aspect ) , nearDist( near ) , farDist( far )
{
	updateClipMatrix();
}

Camera::Camera( const Camera& copy )
	: BaseShape() , hFOV( copy.hFOV ) , aspect( copy.aspect ) , nearDist( copy.nearDist ) , farDist( copy.farDist )
{
	updateClipMatrix();
}

Camera::~Camera()
{
	BaseShape::~BaseShape();
}

void Camera::rotateCameraByLookAt( const Vector3& lookPoint , const Vector3& upDir /*= Vector3( 0.0f , 0.0f , 1.0f ) */ )
{
	Vector3 forwardDir = lookPoint - position;
	forwardDir.normalized();

	Vector3 rightDir = Vector3::cross( upDir , forwardDir );
	rightDir.normalized();

	Vector3 realUpDir = Vector3::cross( forwardDir , rightDir );
	realUpDir.normalized();

	Matrix rotMat( forwardDir , rightDir , realUpDir );
	worldMatrix.updateMatrix( position , rotMat , scaling );

	viewMatrix = worldMatrix;
	viewMatrix.inverse();
}

void Camera::operator=( const Camera& copy )
{
	BaseShape::operator=( copy );

	hFOV = copy.hFOV;
	aspect = copy.aspect;
	nearDist = copy.nearDist;
	farDist = copy.farDist;

	updateClipMatrix();
}

////////////////////////////////////////////////////////////
void Camera::updateClipMatrix()
{
	//view坐标系 向前x 向上z 向右y 

	//裁剪矩阵
	// f/(f-n)  0      0      1
	// 0        zoom_h 0      0
	// 0        0      zoom_v 0
	// nf/(n-f) 0      0      0

	//乘完结果 深度在x上 屏幕坐标在（y，z）

	float tan_half_hfov = tanf( hFOV / 360.0f * PI );
	float tan_half_vfov = tan_half_hfov / aspect;

	clipMatrix( 0 , 0 ) = farDist / ( farDist - nearDist ); 
	clipMatrix( 1 , 1 ) = 1.0f / tan_half_hfov;//zoom_horizen
	clipMatrix( 2 , 2 ) = 1.0f / tan_half_vfov;//zoom_vertical
	clipMatrix( 0 , 3 ) = 1.0f;
	clipMatrix( 3 , 0 ) = nearDist*farDist / ( nearDist - farDist );
}

void Camera::updateWorldMatrix()
{
	BaseShape::updateWorldMatrix();

	viewMatrix = worldMatrix;
	viewMatrix.inverse();
}
