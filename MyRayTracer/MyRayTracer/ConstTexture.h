#pragma once
#include "Texture.h"
#include "Vector3.h"

class ConstTexture : public Texture
{
public:
	ConstTexture();
	ConstTexture( const Vector3 &inCol );

	virtual Vector3 sample( float u , float v , const Vector3 &worldPos ) const override;

private:

	Vector3 color;
};

