#include "Camera.h"
#include <math.h>

#define M_PI       3.1415926f

Camera::Camera( float aspect , float vfov )
	: aspectRatio( aspect ) , vFOV( vfov ) , origin( 0.0f , 0.0f , 0.0f ) , lookPoint( 0.0f , 0.0f , -1.0f ) ,
	focusDistance( 1.0f ) , lensRadius( 0.0f )
{
	calcCameraDetail();
}

Camera::Camera( const Vector3 &pos , const Vector3 &lookAt , float aspect , float vfov ,
	float aperture , float focus_dist )
	: aspectRatio( aspect ) , vFOV( vfov ) , origin( pos ) , lookPoint( lookAt ) , focusDistance( focus_dist ) , 
	lensRadius( aperture * 0.5f )
{
	calcCameraDetail();
}

Ray Camera::getRay( float u , float v ) const
{
	Vector3 randCircle = Vector3::getRandomInDisk() * lensRadius;
	Vector3 offset = randCircle.x() * xAxis + randCircle.y() * yAxis;

	Vector3 rayDir = lowerLeftCorner + horizental * u + vertical * v - origin - offset;
	rayDir.normalized();

	return Ray( origin + offset , rayDir );
}

void Camera::calcCameraDetail()
{
	float theta = vFOV * M_PI / 180.0f;
	float halfHeight = tanf( theta*0.5f ) * focusDistance;
	float halfWidth = aspectRatio * halfHeight;

	zAxis = origin - lookPoint;
	zAxis.normalized();

	xAxis = Vector3::cross( Vector3( 0.0f , 1.0f , 0.0f ) , zAxis );
	xAxis.normalized();

	yAxis = Vector3::cross( zAxis , xAxis );
	yAxis.normalized();

	lowerLeftCorner = origin - xAxis * halfWidth - yAxis * halfHeight - zAxis * focusDistance;
	horizental = xAxis * 2.0f * halfWidth;
	vertical = yAxis * 2.0f * halfHeight;
}
