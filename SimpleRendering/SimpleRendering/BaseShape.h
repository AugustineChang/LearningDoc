#pragma once
#include "Matrix.h"
#include "Vector2.h"

struct Vertex
{
	Vertex() : pos() , normal() , color() , uvCoord()
	{}

	Vector3 pos;
	Vector3 normal;
	Vector3 color;
	Vector2 uvCoord;
};

class BaseShape
{
public:

	BaseShape();
	BaseShape( const BaseShape &copy );
	~BaseShape();
	
	void operator=( const BaseShape &copy );

	void setShapePostion( const Vector3 &pos );
	void setShapeRotation( const Vector3 &rot );
	void setShapeScale( const Vector3 &scale );

	void addShapePostion( const Vector3 &delta );
	void addShapeRotation( const Vector3 &delta );
	void addShapeScale( const Vector3 &delta );

	Vector3 getShapePosition() const 
	{
		return position;
	}

	Vector3 getShapeRotation() const
	{
		return rotation;
	}

	Vector3 getShapeScale() const
	{
		return scaling;
	}

	void setDrawMode( int mode );
	int getDrawMode() const
	{
		return drawMode;
	}

	Vector3 transformPostion( const Vector3& pos );
	Vector3 transformDirection( const Vector3& vec );

	Vector3 inverseTransformPostion( const Vector3& pos );
	Vector3 inverseTransformDirection( const Vector3& vec );

	Vertex operator[]( int index ) const;
	Vertex& operator[]( int index );

	Matrix getWorldMatrix() const
	{
		return worldMatrix;
	}

	Vector3 getForward() const;
	Vector3 getRight() const;

	void getVertice( const Vertex *&vertice , int &num ) const
	{
		vertice = vertexList;
		num = numOfVertice;
	}

	void getIndice( const int *&indice , int &num ) const
	{
		indice = indexList;
		num = numOfIndice;
	}

protected:

	virtual void updateWorldMatrix();

	int numOfVertice;
	Vertex *vertexList;
	int numOfIndice;
	int *indexList;
	int drawMode;//0-framework 1-fill

	Vector3 position;
	Vector3 rotation;
	Vector3 scaling;

	Matrix worldMatrix;
};

