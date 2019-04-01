#include "Camera.h"
#include "MyMath.h"

Camera::Camera( float aspect , float vfov )
	: aspectRatio( aspect ) , vFOV( vfov ) , origin( 0.0f , 0.0f , 0.0f ) , lookPoint( 0.0f , 0.0f , -1.0f ) ,
	focusDistance( 1.0f ) , lensRadius( 0.0f ) , exposureTime( 1.0f )
{
	calcCameraDetail();
}

Camera::Camera( const Vector3 &pos , const Vector3 &lookAt , float aspect , float vfov ,
	float aperture , float focus_dist , float exposure )
	: aspectRatio( aspect ) , vFOV( vfov ) , origin( pos ) , lookPoint( lookAt ) , focusDistance( focus_dist ) , 
	lensRadius( aperture * 0.5f ) , exposureTime( exposure )
{
	calcCameraDetail();
}

Ray Camera::getRay( float u , float v ) const
{
	Vector3 randCircle = Vector3::getRandomInDisk() * lensRadius;
	Vector3 offset = randCircle.x() * xAxis + randCircle.y() * yAxis;

	Vector3 rayDir = lowerLeftCorner + horizental * u + vertical * v - origin - offset;
	rayDir.normalized();

	float randomSendTime = MyMath::getRandom01() * exposureTime;

	return Ray( origin + offset , rayDir , randomSendTime );
}

void Camera::calcCameraDetail()
{
	float theta = vFOV * PI / 180.0f;
	float halfHeight = MyMath::tan( theta*0.5f ) * focusDistance;
	float halfWidth = aspectRatio * halfHeight;

	lensRadius = max( lensRadius , 0.0f );
	focusDistance = max( focusDistance , 0.0f );
	exposureTime = max( exposureTime , 0.0f );

	zAxis = origin - lookPoint;
	zAxis.normalized();

	xAxis = Vector3::cross( Vector3::upVector , zAxis );
	xAxis.normalized();

	yAxis = Vector3::cross( zAxis , xAxis );
	yAxis.normalized();

	lowerLeftCorner = origin - xAxis * halfWidth - yAxis * halfHeight - zAxis * focusDistance;
	horizental = xAxis * 2.0f * halfWidth;
	vertical = yAxis * 2.0f * halfHeight;
}
