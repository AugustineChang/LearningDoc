#pragma once
#include "Texture.h"
#include "MyMath.h"
#include "Vector3.h"

class PerlinNoise
{
public:

	PerlinNoise();

	float getNoise( const Vector3 &pos ) const;
	float getTurbNoise( const Vector3 &pos , float depth = 7 )const;

private:

	void generateRandFloat( float *list , int num );
	void generateRandVector( Vector3 *list , int num );
	void generateRandList( int *list , int num );

	float perlinInterpolate( Vector3( &aroundVec )[2][2][2] , float u , float v , float w ) const;
	float trilinearInterpolate( float( &around )[2][2][2] , float u , float v , float w ) const;

private:

	Vector3 randVec[256];
	int randListX[256];
	int randListY[256];
	int randListZ[256];
};

class PerlinTexture : public Texture
{
public:
	PerlinTexture();

	virtual Vector3 sample( float u , float v , const Vector3 &worldPos ) const override;

private:

	PerlinNoise noise;
	float tilingScale;
};

