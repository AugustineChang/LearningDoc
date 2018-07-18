#include "PPMImage.h"
#include "SimpleMath.h"
#include <fstream>
#include <string>

PPMImage::PPMImage( const char* fileName )
{
	loadImage( fileName );
}

PPMImage::PPMImage( const PPMImage &copy )
{
	width = copy.width;
	height = copy.height;

	int leng = width * height;
	pixelList = new Vector3[leng];
	for ( int i = 0; i < leng; ++i )
	{
		pixelList[i] = copy.pixelList[i];
	}
}

PPMImage::~PPMImage()
{
	delete[] pixelList;
}

Vector3 PPMImage::sampleImage( const Vector2& uv ) const
{
	float coordX = uv[0] * ( width - 1 );
	float coordY = uv[1] * ( height - 1 );
	int curX = SimpleMath::floorToInt( coordX );
	int curY = SimpleMath::floorToInt( coordY );

	return pixelList[curX + curY *width];
}

Vector3 PPMImage::sampleImage2( const Vector2& uv ) const
{
	float coordX = uv[0] * ( width - 1 );
	float coordY = uv[1] * ( height - 1 );
	int curX = SimpleMath::floorToInt( coordX );
	int curY = SimpleMath::floorToInt( coordY );

	float ratioX = coordX - float( curX );
	float ratioY = coordY - float( curY );

	int nextX = ( curX + 1 ) % ( width - 1 );
	int nextY = ( curY + 1 ) % ( height - 1 );

	Vector3 color0 = pixelList[curX + curY *width];
	Vector3 color1 = pixelList[nextX + curY *width];
	Vector3 color2 = pixelList[curX + nextY *width];
	Vector3 color3 = pixelList[nextX + nextY *width];

	Vector3 color01 = SimpleMath::Lerp( color0 , color1 , ratioX );
	Vector3 color23 = SimpleMath::Lerp( color2 , color3 , ratioX );

	return SimpleMath::Lerp( color01 , color23 , ratioY );
}

void PPMImage::loadImage( const char* fileName )
{
	std::ifstream readFile;
	readFile.open( fileName , std::ios::in );
	if ( !readFile.is_open() )
	{
		std::cout << "can not open file:" << fileName << std::endl;
	}

	char lineBuff[100];
	readFile.getline( lineBuff , 100 );
	readFile.getline( lineBuff , 100 );

	readFile >> width >> height;
	int leng = width * height;
	pixelList = new Vector3[leng];

	int maxNum;
	readFile >> maxNum;

	int red , green , blue;
	int counter = 0;
	while ( true )
	{
		readFile >> red >> green >> blue;

		pixelList[counter][0] = float( red ) / float( maxNum );
		pixelList[counter][1] = float( green ) / float( maxNum );
		pixelList[counter][2] = float( blue ) / float( maxNum );
		++counter;

		if ( counter >= leng )
		{
			break;
		}
	}

	readFile.close();
}

void PPMImage::operator=( const PPMImage &copy )
{
	if ( this == &copy ) return;

	int oldLeng = width * height;

	width = copy.width;
	height = copy.height;

	int newLeng = width * height;
	if ( newLeng != oldLeng )
	{
		delete[] pixelList;
		pixelList = new Vector3[newLeng];
	}

	for ( int i = 0; i < newLeng; ++i )
	{
		pixelList[i] = copy.pixelList[i];
	}
}
