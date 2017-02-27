#include <assert.h>
#include "WaveTerrain.h"
#include "../Utilities/CommonHeader.h"

WaveTerrain::WaveTerrain() : SimpleTerrain( { 100,100 } , { 160,160 } ) , dx( 0.8f ) , timeStep( 0.03f ) , speed( 3.25f ) , damping( 0.4f ) ,
timer( 0.0f ) , disturbTimer( 0.0f )
{
	createObjectMesh();

	float d = damping*timeStep + 2.0f;
	float e = ( speed*speed )*( timeStep*timeStep ) / ( dx*dx );
	K1 = ( damping*timeStep - 2.0f ) / d;
	K2 = ( 4.0f - 8.0f*e ) / d;
	K3 = ( 2.0f*e ) / d;

	UINT len = vertices.size();
	for ( const CustomVertex &vert : vertices )
	{
		prevVertice.push_back( vert );
	}
}


WaveTerrain::~WaveTerrain()
{
}

void WaveTerrain::UpdateObject( float DeltaTime )
{
	UpdateDisturb( DeltaTime );

	timer += DeltaTime;
	if ( timer <= timeStep ) return;
	
	UINT numCol = verticesDim.y;

	for ( UINT i = 1; i < verticesDim.x - 1; ++i )
	{
		for ( UINT j = 1; j < verticesDim.y - 1; ++j )
		{
			prevVertice[i*numCol + j].Pos.y =
				K1*prevVertice[i*numCol + j].Pos.y +
				K2*vertices[i*numCol + j].Pos.y +
				K3*( vertices[( i + 1 )*numCol + j].Pos.y +
					vertices[( i - 1 )*numCol + j].Pos.y +
					vertices[i*numCol + j + 1].Pos.y +
					vertices[i*numCol + j - 1].Pos.y );
		}
	}

	std::swap( prevVertice , vertices );
	timer = 0.0f;
}

void WaveTerrain::UpdateDisturb( float DeltaTime )
{
	disturbTimer += DeltaTime;
	if ( disturbTimer <= 0.25f )return;

	UINT i = 5 + rand() % 90;
	UINT j = 5 + rand() % 90;

	float r = SimpleMath::RandF( 1.0f , 2.0f );

	disturb( i , j , r );
	disturbTimer = 0.0f;
}

float WaveTerrain::getHeight( float x , float z , float time ) const
{
	return 0.0f;
}

void WaveTerrain::createObjectMesh()
{
	createBasicPlane();
}

void WaveTerrain::disturb( UINT i , UINT j , float magnitude )
{
	assert( i > 1 && i < verticesDim.x - 2 );
	assert( j > 1 && j < verticesDim.y - 2 );

	float halfMag = 0.5f*magnitude;
	UINT numCol = verticesDim.y;

	// Disturb the ijth vertex height and its neighbors.
	vertices[i*numCol + j].Pos.y += magnitude;
	vertices[i*numCol + j + 1].Pos.y += halfMag;
	vertices[i*numCol + j - 1].Pos.y += halfMag;
	vertices[( i + 1 )*numCol + j].Pos.y += halfMag;
	vertices[( i - 1 )*numCol + j].Pos.y += halfMag;
}
