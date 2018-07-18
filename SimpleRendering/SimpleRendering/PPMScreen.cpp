#include "PPMScreen.h"
#include <fstream>
#include <iostream>

PPMScreen::PPMScreen( int w , int h )
	: width( w + 1 ) , height( h + 1 ) , pixelList( nullptr ) , isOnSSAA( false ) , ssaaMulti( 2 )
{
	updateRealWH();
	int leng = realW * realH;
	pixelList = new Pixel[leng];
}

PPMScreen::PPMScreen( const PPMScreen &copy )
{
	width = copy.width;
	height = copy.height;
	isOnSSAA = copy.isOnSSAA;
	ssaaMulti = copy.ssaaMulti;
	updateRealWH();

	int leng = realW * realH;
	pixelList = new Pixel[leng];
	for ( int i = 0; i < leng; ++i )
	{
		pixelList[i] = copy.pixelList[i];
	}
}

PPMScreen::~PPMScreen()
{
	delete[] pixelList;
}

void PPMScreen::setPixel( int x , int y , const Vector3 &pixel )
{
	if ( x < 0 || y < 0 || x >= realW || y >= realH )
		return;

	Pixel &refPixel = pixelList[x + y * realW];
	refPixel.color[0] = SimpleMath::Clamp01( pixel[0] );
	refPixel.color[1] = SimpleMath::Clamp01( pixel[1] );
	refPixel.color[2] = SimpleMath::Clamp01( pixel[2] );
}

void PPMScreen::setPixel( int x , int y , float red , float green , float blue )
{
	if ( x < 0 || y < 0 || x >= realW || y >= realH )
		return;

	red = SimpleMath::Clamp01( red );
	green = SimpleMath::Clamp01( green );
	blue = SimpleMath::Clamp01( blue );

	Pixel &refPixel = pixelList[x + y * realW];
	refPixel.color[0] = red;
	refPixel.color[1] = green;
	refPixel.color[2] = blue;
}

bool PPMScreen::setDepth( int x , int y , float dep , bool isZWriteOn /*= true */ )
{
	if ( x < 0 || y < 0 || x >= realW || y >= realH )
		return false;

	dep = SimpleMath::Clamp01( dep );

	Pixel &refPixel = pixelList[x + y * realW];
	if ( dep < refPixel.depth )
	{
		if ( isZWriteOn ) refPixel.depth = dep;
		return true;
	}
	else
		return false;
}

void PPMScreen::setSSAAOnOff( bool onOff )
{
	if ( onOff == isOnSSAA ) return;

	isOnSSAA = onOff;
	int oldLeng = realW * realH;
	updateRealWH();

	if ( isOnSSAA )
	{
		int leng = realW * realH;
		int multi = leng / oldLeng;

		Pixel *temp = new Pixel[leng];
		for ( int i = 0; i < leng; ++i )
		{
			temp[i] = pixelList[i / multi];
		}

		delete[] pixelList;
		pixelList = temp;
	}
	else
	{
		int leng = realW * realH;
		int multi = oldLeng / leng;

		Pixel *temp = new Pixel[leng];
		for ( int i = 0; i < leng; ++i )
		{
			temp[i] = pixelList[i * multi];
		}

		delete[] pixelList;
		pixelList = temp;
	}
}

Vector3 PPMScreen::getPixel( int x , int y ) const
{
	x = SimpleMath::Max( x , 0 ) % realW;
	y = SimpleMath::Max( y , 0 ) % realH;

	Vector3 finalColor;
	if ( isOnSSAA )
	{
		int curIndex = ( x + y*realW )*ssaaMulti;

		Pixel &refPixel0 = pixelList[curIndex];
		Pixel &refPixel1 = pixelList[curIndex + 1];
		Pixel &refPixel2 = pixelList[curIndex + realW];
		Pixel &refPixel3 = pixelList[curIndex + realW + 1];

		finalColor += refPixel0.color;
		finalColor += refPixel1.color;
		finalColor += refPixel2.color;
		finalColor += refPixel3.color;

		finalColor /= 4.0f;
	}
	else
	{
		Pixel &refPixel = pixelList[x + y*realW];
		finalColor = refPixel.color;
	}
	return finalColor;
}

int PPMScreen::getImageWidth() const
{
	return realW - 1;
}

int PPMScreen::getImageHeight() const
{
	return realH - 1;
}

void PPMScreen::clearImage()
{
	for ( int y = 0; y < realH; ++y )
	{
		for ( int x = 0; x < realW; ++x )
		{
			Pixel &refPixel = pixelList[x + y*realW];

			for ( int i = 0; i < 4; ++i )
			{
				refPixel.color[0] = 0.0f;
				refPixel.color[1] = 0.0f;
				refPixel.color[2] = 0.0f;
			}
			refPixel.depth = 2.0f;
		}
	}
}

void PPMScreen::renderBackground( const Vector3 &upCol , const Vector3 &downCol )
{
	for ( int y = 0; y < realH; ++y )
	{
		float ratio = float( y ) / float( realH - 1 );
		ratio = SimpleMath::Clamp01( ratio * 1.5f );
		Vector3 color = SimpleMath::Lerp( upCol , downCol , ratio );

		for ( int x = 0; x < realW; ++x )
		{
			Pixel &refPixel = pixelList[x + y*realW];

			refPixel.color = color;
			refPixel.depth = 1.0f;
		}
	}
}

void PPMScreen::saveImage( const std::string &filePath ) const
{
	std::ofstream outputFile;
	outputFile.open( filePath.c_str() , std::ios::out | std::ios::trunc );
	if ( !outputFile.is_open() )
	{
		std::cout << "can not open file:" << filePath << std::endl;
	}

	outputFile << "P3\n" << width << ' ' << height << "\n255\n";
	for ( int y = 0; y < height; ++y )
	{
		for ( int x = 0; x < width; ++x )
		{
			Vector3 finalColor;
			if ( isOnSSAA )
			{
				int curIndex = ( x + y*realW )*ssaaMulti;
				
				Pixel &refPixel0 = pixelList[curIndex];
				Pixel &refPixel1 = pixelList[curIndex + 1];
				Pixel &refPixel2 = pixelList[curIndex + realW];
				Pixel &refPixel3 = pixelList[curIndex + realW + 1];

				finalColor += refPixel0.color;
				finalColor += refPixel1.color;
				finalColor += refPixel2.color;
				finalColor += refPixel3.color;

				finalColor /= 4.0f;
			}
			else
			{
				Pixel &refPixel = pixelList[x + y*realW];
				finalColor = refPixel.color;
			}

			int red = SimpleMath::floorToInt( finalColor[0] * 255.99f );
			int green = SimpleMath::floorToInt( finalColor[1] * 255.99f );
			int blue = SimpleMath::floorToInt( finalColor[2] * 255.99f );

			outputFile << red << ' ' << green << ' ' << blue << '\n';
		}
	}
	outputFile.close();
}

void PPMScreen::saveDepthMap( const std::string &filePath ) const
{
	std::ofstream outputFile;
	outputFile.open( filePath.c_str() , std::ios::out | std::ios::trunc );
	if ( !outputFile.is_open() )
	{
		std::cout << "can not open file:" << filePath << std::endl;
	}

	outputFile << "P2\n" << realW << ' ' << realH << "\n255\n";
	for ( int y = 0; y < realH; ++y )
	{
		for ( int x = 0; x < realW; ++x )
		{
			Pixel &refPixel = pixelList[x + y*realW];
			float finalColor = refPixel.depth;

			int grey = SimpleMath::floorToInt( finalColor * 255.99f );

			outputFile << grey << ' ';
		}
		outputFile << '\n';
	}
	outputFile.close();
}

void PPMScreen::operator=( const PPMScreen &copy )
{
	if ( this == &copy ) return;

	int oldLeng = realW * realH;

	width = copy.width;
	height = copy.height;
	isOnSSAA = copy.isOnSSAA;
	ssaaMulti = copy.ssaaMulti;
	updateRealWH();

	int newLeng = realW * realH;
	if ( newLeng != oldLeng )
	{
		delete[] pixelList;
		pixelList = new Pixel[newLeng];
	}
	
	for ( int i = 0; i < newLeng; ++i )
	{
		pixelList[i] = copy.pixelList[i];
	}
}

void PPMScreen::updateRealWH()
{
	realW = isOnSSAA ? width*ssaaMulti : width;
	realH = isOnSSAA ? height*ssaaMulti : height;
}
