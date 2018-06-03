#pragma once
#include "Vector3.h"
#include "Ray.h"

class Camera
{
public:

	Camera( float aspect );
	Camera( float aspect , const Vector3 &pos );

	Ray getRay( float u , float v )
	{
		return Ray( origin , lowerLeftCorner + horizental * u + vertical * v );
	}

private:

	float aspectRatio;
	Vector3 lowerLeftCorner;
	Vector3 horizental;
	Vector3 vertical;

	Vector3 origin;
};

