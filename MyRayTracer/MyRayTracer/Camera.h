#pragma once
#include "Vector3.h"
#include "Ray.h"

class Camera
{
public:

	Camera( float aspect , float vfov );
	Camera( const Vector3 &pos , const Vector3 &lookAt , float aspect , float vfov , 
		float aperture , float focus_dist , float exposure );

	Ray getRay( float u , float v ) const;
	float getExposureTime() const;

private:

	void calcCameraDetail();

	float aspectRatio;
	float vFOV;
	float focusDistance;
	float lensRadius;
	float exposureTime;

	Vector3 lowerLeftCorner;
	Vector3 horizental;
	Vector3 vertical;

	Vector3 origin;
	Vector3 lookPoint;
	Vector3 upDir;

	Vector3 xAxis , yAxis , zAxis;
};

