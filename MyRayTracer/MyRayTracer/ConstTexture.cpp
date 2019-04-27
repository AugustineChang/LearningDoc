#include "ConstTexture.h"
#include "Vector3.h"


ConstTexture::ConstTexture() : color()
{
}

ConstTexture::ConstTexture( const Vector3 &inCol ) : color( inCol )
{
}

Vector3 ConstTexture::sample( float u , float v , const Vector3 &worldPos ) const
{
	return color;
}


