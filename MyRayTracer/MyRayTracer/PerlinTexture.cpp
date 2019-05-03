#include "PerlinTexture.h"
#include "Vector3.h"

PerlinTexture::PerlinTexture() : noise() , tilingScale( 5.0f )
{
}

Vector3 PerlinTexture::sample( float u , float v , const Vector3 &worldPos ) const
{
	//return Vector3::oneVector * noise.getTurbNoise( worldPos * tilingScale );
	return Vector3::oneVector * 0.5f *( 1.0f + MyMath::sin( tilingScale* worldPos.z() + 10*noise.getTurbNoise( worldPos ) ) );
}

////////////////////////////////////////////////////////////////////////////
//Perlin Noise
////////////////////////////////////////////////////////////////////////////

PerlinNoise::PerlinNoise()
{
	generateRandVector( &randVec[0] , 256 );
	generateRandList( &randListX[0] , 256 );
	generateRandList( &randListY[0] , 256 );
	generateRandList( &randListZ[0] , 256 );
}

float PerlinNoise::getNoise( const Vector3 &pos ) const
{
	float u = pos.x() - floor( pos.x() );
	float v = pos.y() - floor( pos.y() );
	float w = pos.z() - floor( pos.z() );

	int i = static_cast<int>( floor( pos.x() ) ) & 255;
	int j = static_cast<int>( floor( pos.y() ) ) & 255;
	int k = static_cast<int>( floor( pos.z() ) ) & 255;

	Vector3 around[2][2][2];
	for ( int di = 0; di < 2; ++di )
		for ( int dj = 0; dj < 2; ++dj )
			for ( int dk = 0; dk < 2; ++dk )
			{
				int randIndex = randListX[( i + di ) & 255] ^ randListY[( j + dj ) & 255] ^
					randListZ[( k + dk ) & 255];
				around[di][dj][dk] = randVec[randIndex];
			}

	return perlinInterpolate( around , u , v , w );
}

float PerlinNoise::getTurbNoise( const Vector3 &pos , float depth /*= 7 */ ) const
{
	float accum = 0.0f;
	float weight = 1.0f;
	Vector3 tempPos = pos;

	//float sn = ( 1.0f - MyMath::power( 0.5 , depth ) )*2.0f;

	for ( int i = 0; i < depth; ++i )
	{
		accum += getNoise( tempPos ) * weight;
		weight *= 0.5f;
		tempPos *= 2.0f;
	}
	//return accum / sn;
	return MyMath::abs( accum );
}

void PerlinNoise::generateRandFloat( float *list , int num )
{
	for ( int i = 0; i < num; ++i )
	{
		list[i] = MyMath::getRandom01();
	}
}

void PerlinNoise::generateRandVector( Vector3 *list , int num )
{
	for ( int i = 0; i < num; ++i )
	{
		Vector3 randVec = Vector3::getRandomInUnitSphere();
		randVec.normalized();

		list[i] = randVec;
	}
}

void PerlinNoise::generateRandList( int *list , int num )
{
	//1.make a ordered list
	for ( int i = 0; i < num; ++i )
	{
		list[i] = i;
	}

	//2.random permute
	for ( int i = num - 1; i > 0; --i )
	{
		int target = static_cast<int>( MyMath::getRandomInteger( i + 1 ) );
		int temp = list[i];
		list[i] = list[target];
		list[target] = temp;
	}
}

float PerlinNoise::perlinInterpolate( Vector3( &aroundVec )[2][2][2] , float u , float v , float w ) const
{
	float sum = 0.0f;

	//hermite cubic
	float uu = u * u*( 3 - 2 * u );
	float vv = v * v*( 3 - 2 * v );
	float ww = w * w*( 3 - 2 * w );

	for ( int di = 0; di < 2; ++di )
		for ( int dj = 0; dj < 2; ++dj )
			for ( int dk = 0; dk < 2; ++dk )
			{
				Vector3 weightVec( u - di , v - dj , w - dk );
				float dotVal = Vector3::dot( aroundVec[di][dj][dk] , weightVec );

				sum +=
					( di*uu + ( 1 - di )*( 1 - uu ) ) *
					( dj*vv + ( 1 - dj )*( 1 - vv ) ) *
					( dk*ww + ( 1 - dk )*( 1 - ww ) ) *
					dotVal;
					//( dotVal + 1.0f ) * 0.5f;
			}
	return sum;
}

float PerlinNoise::trilinearInterpolate( float( &around )[2][2][2] , float u , float v , float w ) const
{
	float temp[6];
	temp[0] = MyMath::lerp( around[0][0][0] , around[0][0][1] , w );
	temp[1] = MyMath::lerp( around[0][1][0] , around[0][1][1] , w );
	temp[2] = MyMath::lerp( around[1][0][0] , around[1][0][1] , w );
	temp[3] = MyMath::lerp( around[1][1][0] , around[1][1][1] , w );

	temp[4] = MyMath::lerp( temp[0] , temp[1] , v );
	temp[5] = MyMath::lerp( temp[2] , temp[3] , v );

	return MyMath::lerp( temp[4] , temp[5] , u );
}
