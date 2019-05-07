#pragma once
#include "Texture.h"
#include <string>

class ImageTexture : public Texture
{
public:
	ImageTexture( const std::string &filePath );
	ImageTexture( const unsigned char *data , int sizeX , int sizeY , int channel );
	
	virtual Vector3 sample( float u , float v , const Vector3 &worldPos ) const override;

private:

	void loadTexture( const std::string &filePath );

	const unsigned char *imageData;
	int imageSizeX;
	int imageSizeY;
	int imageChannel;
};

