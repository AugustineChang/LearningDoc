#include "ImageTexture.h"
#include "Vector3.h"
#include "MyMath.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

ImageTexture::ImageTexture( const std::string &filePath ) :
	imageData( nullptr ) , imageSizeX( 0 ) , imageSizeY( 0 ) , imageChannel( 0 )
{
	loadTexture( filePath );
}

ImageTexture::ImageTexture( const unsigned char *data , int sizeX , int sizeY , int channel ) :
	imageData( data ) , imageSizeX( sizeX ) , imageSizeY( sizeY ) , imageChannel( channel )
{
}

Vector3 ImageTexture::sample( float u , float v , const Vector3 &worldPos ) const
{
	if ( imageData == nullptr )
		return Vector3::oneVector;
	else
	{
		int i = MyMath::floorToInt( u * imageSizeX );
		int j = MyMath::floorToInt( ( 1.0f - v ) * imageSizeY );
		i = MyMath::clamp( i , 0 , imageSizeX - 1 );
		j = MyMath::clamp( j , 0 , imageSizeY - 1 );

		Vector3 color;
		int pixelIndex = ( i + j * imageSizeX ) * imageChannel;
		
		int readChannel = min( imageChannel , 3 );
		for ( int k = 0; k < readChannel; ++k )
		{
			int val_8bit = static_cast<int>( imageData[pixelIndex + k] );
			color[k] = val_8bit / 255.99f;
		}
		return color;
	}
}

void ImageTexture::loadTexture( const std::string &filePath )
{
	imageData = stbi_load( filePath.c_str() , &imageSizeX , &imageSizeY , &imageChannel , 0 );
}
