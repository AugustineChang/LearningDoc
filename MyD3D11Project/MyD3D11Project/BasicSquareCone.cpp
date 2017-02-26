#include "BasicSquareCone.h"
#include "CommonHeader.h"
using namespace DirectX;


BasicSquareCone::BasicSquareCone() : BasicShape()
{
	createObjectMesh();
}


BasicSquareCone::~BasicSquareCone()
{
}

void BasicSquareCone::createObjectMesh()
{
	XMFLOAT4 redFloat;
	XMStoreFloat4( &redFloat , Colors::Red );
	XMFLOAT4 greenFloat;
	XMStoreFloat4( &greenFloat , Colors::Green );
	XMFLOAT4 blueFloat;
	XMStoreFloat4( &blueFloat , Colors::Blue );
	XMFLOAT4 yellowFloat;
	XMStoreFloat4( &yellowFloat , Colors::Yellow );
	XMFLOAT4 cyanFloat;
	XMStoreFloat4( &cyanFloat , Colors::Cyan );

	vertices =
	{
		{ XMFLOAT3( 1.0f,0.0f,1.0f ), redFloat },
		{ XMFLOAT3( -1.0f,0.0f,1.0f ), greenFloat },
		{ XMFLOAT3( -1.0f,0.0f,-1.0f ), blueFloat },
		{ XMFLOAT3( 1.0f,0.0f,-1.0f ), yellowFloat },
		{ XMFLOAT3( 0.0f,2.0f,0.0f ), cyanFloat }
	};

	indices =
	{
		0, 3, 4, // right
		3, 2, 4, // forward
		2, 1, 4, // left
		1, 0, 4, // back
		2, 3, 1,
		3, 0, 1 // down
	};
}
