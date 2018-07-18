#include "ScreenBuffer.h"



ScreenBuffer::ScreenBuffer( int w , int h )
{
	width = w;
	height = h;

	int leng = width * height;
	colorBuffer = new Byte[leng * 3];
	depthBuffer = new float[leng];
}

ScreenBuffer::~ScreenBuffer()
{
	delete[] colorBuffer;
	delete[] depthBuffer;
}

void ScreenBuffer::setPixel( int x , int y , const Vector3 & pixel )
{
	Byte red = SimpleMath::Clamp( Byte( pixel[0] * 255.99f ) , Byte( 0 ) , Byte( 255 ) );
	Byte green = SimpleMath::Clamp( Byte( pixel[1] * 255.99f ) , Byte( 0 ) , Byte( 255 ) );
	Byte blue = SimpleMath::Clamp( Byte( pixel[2] * 255.99f ) , Byte( 0 ) , Byte( 255 ) );

	int index = ( x + ( height - y - 1 ) * width ) * 3;

	colorBuffer[index] = red;
	colorBuffer[index + 1] = green;
	colorBuffer[index + 2] = blue;
}

bool ScreenBuffer::setDepth( int x , int y , float dep , bool isZWriteOn /*= true */ )
{
	dep = SimpleMath::Clamp01( dep );

	int index = x + ( height - y - 1 ) * width;
	float oldDepth = depthBuffer[index];
	if ( dep < oldDepth )
	{
		if ( isZWriteOn ) depthBuffer[index] = dep;
		return true;
	}
	else
		return false;
}

void ScreenBuffer::clearImage()
{
	int leng = width * height;
	for ( int i = 0; i < leng; ++i )
	{
		int index = i * 3;
		colorBuffer[index] = Byte( 50 );
		colorBuffer[index + 1] = Byte( 50 );
		colorBuffer[index + 2] = Byte( 50 );
		depthBuffer[i] = 1.0f;
	}
}

void ScreenBuffer::renderBackground( int red , int green , int blue )
{
	for ( int y = 0; y < height; ++y )
	{
		float ratio = float(y) / float( height - 1 );
		ratio = SimpleMath::Clamp01( ratio * 1.5f );
		int curRed = SimpleMath::Lerp( red , 255 , ratio );
		int curGreen = SimpleMath::Lerp( green , 255 , ratio );
		int curBlue = SimpleMath::Lerp( blue , 255 , ratio );

		for ( int x = 0; x < width; ++x )
		{
			int index = x + ( height - y - 1 ) * width;
			int index3 = index * 3;

			colorBuffer[index3] = curRed;
			colorBuffer[index3 + 1] = curGreen;
			colorBuffer[index3 + 2] = curBlue;

			depthBuffer[index] = 1.0f;
		}
	}
}
