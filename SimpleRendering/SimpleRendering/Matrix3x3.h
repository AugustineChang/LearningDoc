#pragma once
#include "Matrix.h"

class Matrix3x3
{
public:
	Matrix3x3( const Matrix& mat4x4 , int removeI , int removeJ );

	float operator()( int i , int j ) const
	{
		return mat[i][j];
	}

	float& operator()( int i , int j )
	{
		return mat[i][j];
	}

	float deteminant();
	bool inverse();

	friend std::ostream& operator<<( std::ostream& out , const Matrix3x3& m );

private:

	float mat[3][3];
};

std::ostream& operator<<( std::ostream& out , const Matrix3x3& m );