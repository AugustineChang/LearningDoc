#pragma once
#include <string>
#include "Vector3.h"
#include "SimpleMath.h"

struct Pixel 
{
	Vector3 color;
	float depth;
};

class PPMScreen
{
public:
	PPMScreen( int w , int h );
	PPMScreen( const PPMScreen &copy );
	~PPMScreen();

	void operator=( const PPMScreen &copy );

	void setPixel( int x , int y , const Vector3 & pixel );
	void setPixel( int x , int y , float red , float green , float blue );
	bool setDepth( int x , int y , float dep , bool isZWriteOn = true );

	void setSSAAOnOff( bool onOff );
	bool getSSAAOnOff() const
	{
		return isOnSSAA;
	}

	Vector3 getPixel( int x , int y ) const;

	int getImageWidth() const;
	int getImageHeight() const;

	int getSSAAMulti() const
	{
		return ssaaMulti;
	}

	void clearImage();
	void renderBackground( const Vector3 &upCol , const Vector3 &downCol );

	void saveImage( const std::string &filePath ) const;
	void saveDepthMap( const std::string &filePath ) const;
private:

	void updateRealWH();

	Pixel *pixelList;

	int width;
	int height;
	bool isOnSSAA;
	int ssaaMulti;

	int realW;
	int realH;
};

