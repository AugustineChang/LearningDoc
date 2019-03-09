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

float CubicInterpolate( float ym1 , float y0 , float y1 , float y2 , float x )
{
	float b0 = 0 * ym1 + 6 * y0 + 0 * y1 + 0 * y2;
	float b1 = -2 * ym1 - 3 * y0 + 6 * y1 - 1 * y2;
	float b2 = 3 * ym1 - 6 * y0 + 3 * y1 + 0 * y2;
	float b3 = -1 * ym1 + 3 * y0 - 3 * y1 + 1 * y2;
	float x2 = x * x;
	float x3 = x2 * x;
	return 1.f / 6.f * ( b0 + x * b1 + x2 * b2 + x3 * b3 );
}

void PPMImage::CreateNoiseTexture( UINT width , UINT height , UINT halfDepth )
{
	UINT depth = halfDepth * halfDepth;
	float* NoiseData = new float[width * height * depth];

	#define NOISE(i,j,k) NoiseData[i + j * width + k * (width * height)]
	#define max(a,b) (((a) > (b)) ? (a) : (b))
	#define min(a,b) (((a) < (b)) ? (a) : (b))

	// Populate texture with random noise
	UINT InitialStep = 8;
	for ( UINT i = 0; i < width; i += InitialStep )
		for ( UINT j = 0; j < height; j += InitialStep )
			for ( UINT k = 0; k < depth; k += InitialStep )
				NOISE( i , j , k ) = (float) rand() / (float) RAND_MAX;

	// Smooth rows
	for ( UINT i = 0; i < width; ++i )
		for ( UINT j = 0; j < height; j += InitialStep )
			for ( UINT k = 0; k < depth; k += InitialStep )
			{
				int i0 = ( i / InitialStep )*InitialStep;
				int im1 = i0 - InitialStep;
				if ( im1 < 0 )im1 += width;
				int i1 = ( i0 + InitialStep ) % width;
				int i2 = ( i0 + 2 * InitialStep ) % width;
				NOISE( i , j , k ) = CubicInterpolate( NOISE( im1 , j , k ) , NOISE( i0 , j , k ) , NOISE( i1 , j , k ) , NOISE( i2 , j , k ) , (float) ( i - i0 ) / (float) InitialStep );
			}

	// Smooth columns
	for ( UINT i = 0; i < width; ++i )
		for ( UINT j = 0; j < height; ++j )
			for ( UINT k = 0; k < depth; k += InitialStep )
			{
				int j0 = ( j / InitialStep )*InitialStep;
				int jm1 = j0 - InitialStep;
				if ( jm1 < 0 )jm1 += height;
				int j1 = ( j0 + InitialStep ) % height;
				int j2 = ( j0 + 2 * InitialStep ) % height;
				NOISE( i , j , k ) = CubicInterpolate( NOISE( i , jm1 , k ) , NOISE( i , j0 , k ) , NOISE( i , j1 , k ) , NOISE( i , j2 , k ) , (float) ( j - j0 ) / (float) InitialStep );
			}

	// Smooth in depth direction
	for ( UINT i = 0; i < width; ++i )
		for ( UINT j = 0; j < height; ++j )
			for ( UINT k = 0; k < depth; ++k )
			{
				int k0 = ( k / InitialStep )*InitialStep;
				int km1 = k0 - InitialStep;
				if ( km1 < 0 )km1 += depth;
				int k1 = ( k0 + InitialStep ) % depth;
				int k2 = ( k0 + 2 * InitialStep ) % depth;
				NOISE( i , j , k ) = CubicInterpolate( NOISE( i , j , km1 ) , NOISE( i , j , k0 ) , NOISE( i , j , k1 ) , NOISE( i , j , k2 ) , (float) ( k - k0 ) / (float) InitialStep );
			}

	///Save Noise Tex
	std::ofstream writeFile;
	writeFile.open( "noise.pbm" , std::ios::out | std::ios::trunc );

	writeFile << "P1\n" << width * halfDepth << ' ' << height * halfDepth << "\n255\n";
	for ( UINT d1 = 0; d1 < halfDepth; ++d1 )
	{
		for ( UINT y = 0; y < height; ++y )
		{
			for ( UINT d2 = 0; d2 < halfDepth; ++d2 )
			{
				UINT d = d1 * halfDepth + d2;
				for ( UINT x = 0; x < width; ++x )
				{
					float &refPixel = NoiseData[d*height*width + y * width + x];
					writeFile << min( max( (int) ( refPixel*255.f ) , 0 ) , 255 ) << '\n';
				}
			}
		}
	}

	writeFile.close();
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
