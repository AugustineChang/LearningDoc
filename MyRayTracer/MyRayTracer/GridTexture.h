#pragma once
#include "Texture.h"
#include "Vector3.h"

class GridTexture : public Texture
{
public:
	GridTexture();
	GridTexture( float size , const Vector3 & col1 , const Vector3 & col2 );

	virtual Vector3 sample( float u , float v , const Vector3 &worldPos ) const override;

private:

	float gridSize;
	Vector3 gridColor1;
	Vector3 gridColor2;
};

