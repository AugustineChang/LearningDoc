#include "Camera.h"


Camera::Camera( float aspect )
	: aspectRatio( aspect ) , origin( 0.0f , 0.0f , 0.0f ) , vertical( 0.0f , 2.0f , 0.0f )
{
	lowerLeftCorner = Vector3( -aspectRatio , -1.0f , -1.0f );
	horizental = Vector3( 2.0f * aspectRatio , 0.0f , 0.0f );
}

Camera::Camera( float aspect , const Vector3 &pos )
	: aspectRatio( aspect ) , origin( pos ) , vertical( 0.0f , 2.0f , 0.0f )
{
	lowerLeftCorner = Vector3( -aspectRatio , -1.0f , -1.0f );
	horizental = Vector3( 2.0f * aspectRatio , 0.0f , 0.0f );
}
