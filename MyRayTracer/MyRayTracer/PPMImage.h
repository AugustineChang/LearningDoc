#pragma once
#include <string>

typedef unsigned char UINT8;
struct PPMPixel 
{
	PPMPixel() : R( 0 ) , G( 0 ) , B( 0 )
	{
	}

	UINT8 R;
	UINT8 G;
	UINT8 B;
};

class PPMImage
{
public:
	PPMImage();
	PPMImage( int w , int h );
	PPMImage( const PPMImage& copy );

	~PPMImage();
	void operator=( const PPMImage& copy );
	
	void SetPixel( int x , int y , PPMPixel pixel );
	void SetPixel( int x , int y , UINT8 R , UINT8 G , UINT8 B );
	PPMPixel GetPixel( int x , int y ) const;

	void SaveImage( const std::string &filePath );
	void LoadImage( const std::string &filePath );

private:

	void createImage();

	PPMPixel *imageData;

	int width;
	int height;
};

