#include "BaseShape.h"



BaseShape::BaseShape()
	: position() , rotation() , scaling( 1.0f , 1.0f , 1.0f ) , numOfVertice( 0 ) , vertexList( nullptr ) ,
	numOfIndice( 0 ) , indexList( nullptr ) , drawMode( 1 )
{
}


BaseShape::BaseShape( const BaseShape &copy )
	: position( copy.position ) , rotation( copy.rotation ) , scaling( copy.scaling ) , numOfVertice( copy.numOfVertice ) ,
	numOfIndice( copy.numOfIndice )
{
	if ( numOfVertice > 0 )
	{
		vertexList = new Vertex[numOfVertice];
		for ( int i = 0; i < numOfVertice; ++i )
		{
			vertexList[i] = copy.vertexList[i];
		}
	}

	if ( numOfIndice > 0 )
	{
		indexList = new int[numOfIndice];
		for ( int i = 0; i < numOfIndice; ++i )
		{
			indexList[i] = copy.indexList[i];
		}
	}

	updateWorldMatrix();
}

BaseShape::~BaseShape()
{
	if ( numOfIndice > 0 )
	{
		delete[] indexList;
	}

	if ( numOfVertice > 0 )
	{
		delete[] vertexList;
	}
}

void BaseShape::operator=( const BaseShape &copy )
{
	if ( numOfIndice > 0 )
	{
		delete[] indexList;
	}

	if ( numOfVertice > 0 )
	{
		delete[] vertexList;
	}

	numOfIndice = copy.numOfIndice;
	numOfVertice = copy.numOfVertice;
	position = copy.position;
	rotation = copy.rotation;
	scaling = copy.scaling;
	updateWorldMatrix();

	if ( numOfVertice > 0 )
	{
		vertexList = new Vertex[numOfVertice];
		for ( int i = 0; i < numOfVertice; ++i )
		{
			vertexList[i] = copy.vertexList[i];
		}
	}

	if ( numOfIndice > 0 )
	{
		indexList = new int[numOfIndice];
		for ( int i = 0; i < numOfIndice; ++i )
		{
			indexList[i] = copy.indexList[i];
		}
	}
}

void BaseShape::setShapePostion( const Vector3 &pos )
{
	position = pos;
	updateWorldMatrix();
}

void BaseShape::setShapeRotation( const Vector3 &rot )
{
	rotation = rot;

	if ( rotation[0] > 90.0f ) rotation[0] -= 90.0f;
	if ( rotation[1] > 180.0f ) rotation[1] -= 180.0f;
	if ( rotation[2] > 180.0f ) rotation[2] -= 180.0f;
	if ( rotation[0] < -90.0f ) rotation[0] += 90.0f;
	if ( rotation[1] < -180.0f ) rotation[1] += 180.0f;
	if ( rotation[2] < -180.0f ) rotation[2] += 180.0f;

	updateWorldMatrix();
}

void BaseShape::setShapeScale( const Vector3 &scale )
{
	scaling = scale;
	updateWorldMatrix();
}

void BaseShape::addShapePostion( const Vector3 &delta )
{
	position += delta;
	updateWorldMatrix();
}

void BaseShape::addShapeRotation( const Vector3 &delta )
{
	rotation += delta;

	if ( rotation[0] > 90.0f ) rotation[0] -= 90.0f;
	if ( rotation[1] > 90.0f ) rotation[1] -= 90.0f;
	if ( rotation[2] > 180.0f ) rotation[2] -= 180.0f;
	if ( rotation[0] < -90.0f ) rotation[0] += 90.0f;
	if ( rotation[1] < -90.0f ) rotation[1] += 90.0f;
	if ( rotation[2] < -180.0f ) rotation[2] += 180.0f;

	updateWorldMatrix();
}

void BaseShape::addShapeScale( const Vector3 &delta )
{
	scaling += delta;
	updateWorldMatrix();
}

void BaseShape::setDrawMode( int mode )
{
	if ( mode < 0 || mode > 1 )return;

	drawMode = mode;
}

Vector3 BaseShape::transformPostion( const Vector3& pos )
{
	return ( Vector4( pos , 1.0f ) * worldMatrix ).getVector3();
}

Vector3 BaseShape::transformDirection( const Vector3& vec )
{
	return ( Vector4( vec , 0.0f ) * worldMatrix ).getVector3();
}

Vector3 BaseShape::inverseTransformPostion( const Vector3& pos )
{
	Matrix localMat( worldMatrix );
	localMat.inverse();

	return ( Vector4( pos , 1.0f ) * localMat ).getVector3();
}

Vector3 BaseShape::inverseTransformDirection( const Vector3& vec )
{
	Matrix localMat( worldMatrix );
	localMat.inverse();

	return ( Vector4( vec , 0.0f ) * localMat ).getVector3();
}

Vertex BaseShape::operator[]( int index ) const
{
	return vertexList[index];
}

Vertex& BaseShape::operator[]( int index )
{
	return vertexList[index];
}

Vector3 BaseShape::getForward() const
{
	return Vector3( worldMatrix( 0 , 0 ) , worldMatrix( 0 , 1 ) , worldMatrix( 0 , 2 ) );
}

Vector3 BaseShape::getRight() const
{
	return Vector3( worldMatrix( 1 , 0 ) , worldMatrix( 1 , 1 ) , worldMatrix( 1 , 2 ) );
}

////////////////////////////////////////////////////////////////////
void BaseShape::updateWorldMatrix()
{
	worldMatrix.updateMatrix( position , rotation , scaling );
}
