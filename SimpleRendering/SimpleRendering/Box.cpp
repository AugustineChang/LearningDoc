#include "Box.h"

Box::Box() : BaseShape()
{
	numOfVertice = 24;
	numOfIndice = 36;

	vertexList = new Vertex[numOfVertice];
	indexList = new int[numOfIndice]
	{
		0 , 3 , 2 ,
		0 , 2 , 1 , //up
		16 , 19 , 18 ,
		16 , 18 , 17 , //near
		8 , 11 , 10 ,
		8 , 10 , 9 , //right
		4 , 5 , 6 ,
		4 , 6 , 7 , //down
		20 , 21 , 22 ,
		20 , 22 , 23 , //far
		12 , 14 , 15 ,
		12 , 13 , 14   //left
	};

	//定义1x1x1的盒子 0,0,0在中心
	vertexList[0].pos = Vector3( 0.5f , -0.5f , 0.5f );
	vertexList[1].pos = Vector3( -0.5f , -0.5f , 0.5f );
	vertexList[2].pos = Vector3( -0.5f , 0.5f , 0.5f );
	vertexList[3].pos = Vector3( 0.5f , 0.5f , 0.5f );//up
	vertexList[4].pos = Vector3( 0.5f , -0.5f , -0.5f );
	vertexList[5].pos = Vector3( -0.5f , -0.5f , -0.5f );
	vertexList[6].pos = Vector3( -0.5f , 0.5f , -0.5f );
	vertexList[7].pos = Vector3( 0.5f , 0.5f , -0.5f );//down

	vertexList[0].color = Vector3( 1.0f , 1.0f , 1.0f );
	vertexList[1].color = Vector3( 1.0f , 1.0f , 1.0f );
	vertexList[2].color = Vector3( 1.0f , 1.0f , 1.0f );
	vertexList[3].color = Vector3( 1.0f , 1.0f , 1.0f );
	vertexList[4].color = Vector3( 1.0f , 1.0f , 1.0f );
	vertexList[5].color = Vector3( 1.0f , 1.0f , 1.0f );
	vertexList[6].color = Vector3( 1.0f , 1.0f , 1.0f );
	vertexList[7].color = Vector3( 1.0f , 1.0f , 1.0f );

	vertexList[0].uvCoord = Vector2( 0.0f , 0.0f );
	vertexList[1].uvCoord = Vector2( 0.0f , 1.0f );
	vertexList[2].uvCoord = Vector2( 1.0f , 1.0f );
	vertexList[3].uvCoord = Vector2( 1.0f , 0.0f );
	vertexList[4].uvCoord = Vector2( 0.0f , 0.0f );
	vertexList[5].uvCoord = Vector2( 0.0f , 1.0f );
	vertexList[6].uvCoord = Vector2( 1.0f , 1.0f );
	vertexList[7].uvCoord = Vector2( 1.0f , 0.0f );

	vertexList[0].normal = Vector3( 0.0f , 0.0f , 1.0f );
	vertexList[1].normal = Vector3( 0.0f , 0.0f , 1.0f );
	vertexList[2].normal = Vector3( 0.0f , 0.0f , 1.0f );
	vertexList[3].normal = Vector3( 0.0f , 0.0f , 1.0f );
	vertexList[4].normal = Vector3( 0.0f , 0.0f , -1.0f );
	vertexList[5].normal = Vector3( 0.0f , 0.0f , -1.0f );
	vertexList[6].normal = Vector3( 0.0f , 0.0f , -1.0f );
	vertexList[7].normal = Vector3( 0.0f , 0.0f , -1.0f );

	vertexList[8] = vertexList[2];
	vertexList[9] = vertexList[6];
	vertexList[10] = vertexList[7];
	vertexList[11] = vertexList[3];//right
	vertexList[8].normal = Vector3( 0.0f , 1.0f , 0.0f );
	vertexList[9].normal = Vector3( 0.0f , 1.0f , 0.0f );
	vertexList[10].normal = Vector3( 0.0f , 1.0f , 0.0f );
	vertexList[11].normal = Vector3( 0.0f , 1.0f , 0.0f );
	vertexList[8].uvCoord = Vector2( 0.0f , 0.0f );
	vertexList[9].uvCoord = Vector2( 0.0f , 1.0f );
	vertexList[10].uvCoord = Vector2( 1.0f , 1.0f );
	vertexList[11].uvCoord = Vector2( 1.0f , 0.0f );

	vertexList[12] = vertexList[1];
	vertexList[13] = vertexList[5];
	vertexList[14] = vertexList[4];
	vertexList[15] = vertexList[0];//left
	vertexList[12].normal = Vector3( 0.0f , -1.0f , 0.0f );
	vertexList[13].normal = Vector3( 0.0f , -1.0f , 0.0f );
	vertexList[14].normal = Vector3( 0.0f , -1.0f , 0.0f );
	vertexList[15].normal = Vector3( 0.0f , -1.0f , 0.0f );
	vertexList[12].uvCoord = Vector2( 1.0f , 0.0f );
	vertexList[13].uvCoord = Vector2( 1.0f , 1.0f );
	vertexList[14].uvCoord = Vector2( 0.0f , 1.0f );
	vertexList[15].uvCoord = Vector2( 0.0f , 0.0f );

	vertexList[16] = vertexList[1];
	vertexList[17] = vertexList[5];
	vertexList[18] = vertexList[6];
	vertexList[19] = vertexList[2];//near
	vertexList[16].normal = Vector3( -1.0f , 0.0f , 0.0f );
	vertexList[17].normal = Vector3( -1.0f , 0.0f , 0.0f );
	vertexList[18].normal = Vector3( -1.0f , 0.0f , 0.0f );
	vertexList[19].normal = Vector3( -1.0f , 0.0f , 0.0f );
	vertexList[16].uvCoord = Vector2( 0.0f , 0.0f );
	vertexList[17].uvCoord = Vector2( 0.0f , 1.0f );
	vertexList[18].uvCoord = Vector2( 1.0f , 1.0f );
	vertexList[19].uvCoord = Vector2( 1.0f , 0.0f );

	vertexList[20] = vertexList[0];
	vertexList[21] = vertexList[4];
	vertexList[22] = vertexList[7];
	vertexList[23] = vertexList[3];//far
	vertexList[20].normal = Vector3( 1.0f , 0.0f , 0.0f );
	vertexList[21].normal = Vector3( 1.0f , 0.0f , 0.0f );
	vertexList[22].normal = Vector3( 1.0f , 0.0f , 0.0f );
	vertexList[23].normal = Vector3( 1.0f , 0.0f , 0.0f );
	vertexList[20].uvCoord = Vector2( 1.0f , 0.0f );
	vertexList[21].uvCoord = Vector2( 1.0f , 1.0f );
	vertexList[22].uvCoord = Vector2( 0.0f , 1.0f );
	vertexList[23].uvCoord = Vector2( 0.0f , 0.0f );

}