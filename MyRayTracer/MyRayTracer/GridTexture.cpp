#include "GridTexture.h"
#include "Vector3.h"
#include "MyMath.h"

GridTexture::GridTexture() :
	gridSize( 1.0f ) ,
	gridColor1( Vector3::oneVector ) ,
	gridColor2( Vector3::zeroVector )
{
}

GridTexture::GridTexture( float size , const Vector3 & col1 , const Vector3 & col2 ) :
	gridSize( size ) ,
	gridColor1( col1 ) ,
	gridColor2( col2 )
{
}

Vector3 GridTexture::sample( float u , float v , const Vector3 &worldPos ) const
{
	int gridX = MyMath::floorToInt( worldPos.x() / gridSize );
	int gridY = MyMath::floorToInt( worldPos.y() / gridSize );
	int gridZ = MyMath::floorToInt( worldPos.z() / gridSize );

	int sum = gridX + gridY + gridZ;
	if ( sum % 2 == 0 )
	{
		return gridColor1;
	}
	else
	{
		return gridColor2;
	}
}