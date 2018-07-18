#include "Matrix3x3.h"
#include <iomanip>

Matrix3x3::Matrix3x3( const Matrix& mat4x4 , int removeI , int removeJ )
{
	int selfI = 0 , selfJ = 0;
	for ( int i = 0; i < 4; ++i )
	{
		if ( i == removeI ) continue;

		for ( int j = 0; j < 4; ++j )
		{
			if ( j == removeJ ) continue;

			mat[selfI][selfJ] = mat4x4( i , j );
			++selfJ;
		}
		selfJ = 0;
		++selfI;
	}
}

float Matrix3x3::deteminant()
{
	return mat[0][0] * ( mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1] )
		+ mat[0][1] * ( mat[1][2] * mat[2][0] - mat[1][0] * mat[2][2] )
		+ mat[0][2] * ( mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0] );
}

bool Matrix3x3::inverse()
{
	float detM = deteminant();

	if ( detM < 0.00001f ) return false;

	float invDet = 1.0f / detM;

	float detC11 = mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1];
	float detC21 = mat[1][2] * mat[2][0] - mat[1][0] * mat[2][2];
	float detC31 = mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0];
	float detC12 = mat[0][2] * mat[2][1] - mat[0][1] * mat[2][2];
	float detC22 = mat[0][0] * mat[2][2] - mat[0][2] * mat[2][0];
	float detC32 = mat[0][1] * mat[2][0] - mat[0][0] * mat[2][1];
	float detC13 = mat[0][1] * mat[1][2] - mat[0][2] * mat[1][1];
	float detC23 = mat[0][2] * mat[1][0] - mat[0][0] * mat[1][2];
	float detC33 = mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];

	mat[0][0] = detC11 * invDet;
	mat[0][1] = detC12 * invDet;
	mat[0][2] = detC13 * invDet;
	mat[1][0] = detC21 * invDet;
	mat[1][1] = detC22 * invDet;
	mat[1][2] = detC23 * invDet;
	mat[2][0] = detC31 * invDet;
	mat[2][1] = detC32 * invDet;
	mat[2][2] = detC33 * invDet;

	return true;
}

std::ostream& operator<<( std::ostream& out , const Matrix3x3& m )
{
	out << std::setprecision( 3 );

	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 3; ++j )
		{
			out << std::setfill( ' ' ) << std::setw( 10 ) << m.mat[i][j];
		}
		out << std::endl;
	}

	return out;
}