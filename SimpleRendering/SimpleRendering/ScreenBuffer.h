#pragma once
#include "SimpleMath.h"

class ScreenBuffer
{
public:
	ScreenBuffer( int w , int h );
	ScreenBuffer( const ScreenBuffer &copy ) = delete;
	~ScreenBuffer();

	void operator=( const ScreenBuffer &copy ) = delete;

	void setPixel( int x , int y , const Vector3 & pixel );
	bool setDepth( int x , int y , float dep , bool isZWriteOn = true );

	Byte *getColorBuffer() const { return colorBuffer; }

	int getImageWidth() const { return width; }
	int getImageHeight() const{ return height; }

	void clearImage();
	void renderBackground( int red , int green , int blue );

private:

	int width;
	int height;

	Byte *colorBuffer;
	float *depthBuffer;
};

