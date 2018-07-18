#pragma once
#include "Vector2.h"
#include "Vector3.h"

class PPMImage
{
public:
	PPMImage( const char* fileName );
	PPMImage( const PPMImage &copy );
	~PPMImage();

	void operator=( const PPMImage &copy );

	Vector3 sampleImage( const Vector2& uv ) const;
	Vector3 sampleImage2( const Vector2& uv ) const;

private:

	void loadImage( const char* fileName );

	Vector3 *pixelList;

	int width;
	int height;
};