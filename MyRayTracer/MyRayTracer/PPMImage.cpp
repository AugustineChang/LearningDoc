#include "PPMImage.h"
#include <iostream>
#include <fstream>
#include <string>


PPMImage::PPMImage() : width( 100 ) , height( 100 ) , imageData( nullptr )
{
	createImage();
}


PPMImage::PPMImage( int w , int h ) : width( w ) , height( h ) , imageData( nullptr )
{
	createImage();
}

PPMImage::PPMImage( const PPMImage& copy )
{
	if ( copy.width * copy.height != width * height )
	{
		this->width = copy.width;
		this->height = copy.height;
		createImage();
	}
	else
	{
		this->width = copy.width;
		this->height = copy.height;
	}

	int len = width * height;
	for ( int i = 0; i < len; ++i )
	{
		imageData[i] = copy.imageData[i];
	}
}

PPMImage::~PPMImage()
{
	delete[] imageData;
	imageData = nullptr;
}

void PPMImage::operator=( const PPMImage& copy )
{
	if ( &copy == this ) return;

	if ( copy.width * copy.height != width * height )
	{
		this->width = copy.width;
		this->height = copy.height;
		createImage();
	}
	else
	{
		this->width = copy.width;
		this->height = copy.height;
	}

	int len = width * height;
	for ( int i = 0; i < len; ++i )
	{
		imageData[i] = copy.imageData[i];
	}
}

void PPMImage::SetPixel( int x , int y , PPMPixel pixel )
{
	PPMPixel &refPixel = imageData[y*width + x];
	refPixel.R = pixel.R;
	refPixel.G = pixel.G;
	refPixel.B = pixel.B;
}

void PPMImage::SetPixel( int x , int y , UINT8 R , UINT8 G , UINT8 B )
{
	PPMPixel &refPixel = imageData[y*width + x];
	refPixel.R = R;
	refPixel.G = G;
	refPixel.B = B;
}

PPMPixel PPMImage::GetPixel( int x , int y ) const
{
	return imageData[y*width + x];
}

void PPMImage::SaveImage( const std::string &filePath )
{
	std::ofstream writeFile;
	writeFile.open( filePath.c_str() , std::ios::out | std::ios::trunc );

	writeFile << "P3\n" << width << ' ' << height << "\n255\n";
	for ( int y = 0; y < height; ++y )
	{
		for ( int x = 0; x < width; ++x )
		{
			PPMPixel &refPixel = imageData[y*width + x];
			writeFile << int( refPixel.R ) << ' ' << int( refPixel.G ) << ' ' 
				<< int( refPixel.B ) << '\n';
		}
	}

	writeFile.close();
}

void PPMImage::LoadImage( const std::string &filePath )
{
	//std::ifstream readFile;
	//readFile.open( filePath , std::ios::in );

	//char readBuffer[20];
	//readFile.getline( readBuffer , 20 );//P3
	//readFile.getline( readBuffer , 20 );//w h
	//std::string w_h( readBuffer );
	//int index = w_h.find( ' ' );

	//while ( !readFile.eof() )
	//{
	//	readFile.getline( readBuffer , 20 );
	//	std::string curLine( readBuffer );


	//}
}

void PPMImage::createImage()
{
	if ( imageData != nullptr )
	{
		delete[] imageData;
	}

	imageData = new PPMPixel[width * height];
}
