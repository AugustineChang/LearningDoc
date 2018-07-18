#include "Triangle.h"


Triangle::Triangle() : BaseShape()
{
	numOfVertice = 3;
	numOfIndice = 3;

	vertexList = new Vertex[3];
	indexList = new int[3]{ 0,1,2 };

	//定义正三角形 local xoy平面
	vertexList[0].pos = Vector3( 1.0f , 0.0f , 0.0f );
	vertexList[1].pos = Vector3( -0.5f , 0.866025f , 0.0f );
	vertexList[2].pos = Vector3( -0.5f , -0.866025f , 0.0f );

	vertexList[0].color = Vector3( 1.0f , 0.0f , 0.0f );
	vertexList[1].color = Vector3( 0.0f , 1.0f , 0.0f );
	vertexList[2].color = Vector3( 0.0f , 0.0f , 1.0f );

	vertexList[0].normal = Vector3( 0.0f , 0.0f , 1.0f );
	vertexList[1].normal = Vector3( 0.0f , 0.0f , 1.0f );
	vertexList[2].normal = Vector3( 0.0f , 0.0f , 1.0f );

	vertexList[0].uvCoord = Vector2( 0.5f , 0.0f );
	vertexList[1].uvCoord = Vector2( 1.0f , 1.0f );
	vertexList[2].uvCoord = Vector2( 0.0f , 1.0f );
}