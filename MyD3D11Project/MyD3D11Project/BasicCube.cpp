#include "BasicCube.h"
#include "CommonHeader.h"
using namespace DirectX;


BasicCube::BasicCube() : BasicShape()
{
	createObjectMesh();
}


BasicCube::~BasicCube()
{
}

void BasicCube::createObjectMesh()
{
	XMFLOAT4 whiteFloat;
	XMStoreFloat4( &whiteFloat , Colors::White );
	XMFLOAT4 blackFloat;
	XMStoreFloat4( &blackFloat , Colors::Black );
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
	XMFLOAT4 magentaFloat;
	XMStoreFloat4( &magentaFloat , Colors::Magenta );

	vertices =
	{
		{ XMFLOAT3( 1.0f,1.0f,1.0f ), whiteFloat },
		{ XMFLOAT3( -1.0f,1.0f,1.0f ), blackFloat },
		{ XMFLOAT3( 1.0f,1.0f,-1.0f ), redFloat },
		{ XMFLOAT3( -1.0f,1.0f,-1.0f ), greenFloat },
		{ XMFLOAT3( 1.0f,-1.0f,1.0f ), blueFloat },
		{ XMFLOAT3( -1.0f,-1.0f,1.0f ), yellowFloat },
		{ XMFLOAT3( 1.0f,-1.0f,-1.0f ), cyanFloat },
		{ XMFLOAT3( -1.0f,-1.0f,-1.0f ), magentaFloat }
	};

	indices =
	{
		0, 2, 3,
		0, 3, 1, // up
		0, 6, 2,
		0, 4, 6, // right
		0, 1, 5,
		0, 5, 4, // forward
		1, 3, 7,
		1, 7, 5, // left
		3, 2, 6,
		3, 6, 7, // back
		5, 7, 6,
		4, 5, 6  // down
	};
}
