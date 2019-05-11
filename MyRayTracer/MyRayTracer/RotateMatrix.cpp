#include "RotateMatrix.h"
#include "Vector3.h"
#include "MyMath.h"

RotateMatrix::RotateMatrix()
{
	mat[0][0] = 1.0f;
	mat[0][1] = 0.0f;
	mat[0][2] = 0.0f;
	mat[1][0] = 0.0f;
	mat[1][1] = 1.0f;
	mat[1][2] = 0.0f;
	mat[2][0] = 0.0f;
	mat[2][1] = 0.0f;
	mat[2][2] = 1.0f;
}

RotateMatrix::RotateMatrix( const Vector3& eular )
{
	//旋转顺序 roll->pitch->yaw

	//roll z轴
	// cosR sinR 0
	//-sinR cosR 0
	// 0    0    1

	//pitch x轴
	// 1   0    0		// cosR	 sinRcosP  sinRsinP
	// 0   cosP sinP	//-sinR  cosRcosP  cosRsinP
	// 0  -sinP cosP	// 0	-sinP	   cosP

	//yaw y轴
	// cosY 0  -sinY	
	// 0    1   0		
	// sinY 0   cosY


	//结果
	// cosRcosY+sinRsinPsinY	sinRcosP	-cosRsinY+sinRsinPcosY
	//-sinRcosY+cosRsinPsinY	cosRcosP	 sinRsinY+cosRsinPcosY
	// cosPsinY				   -sinP		 cosPcosY

	float eularX = eular.x() / 180.0f * PI;
	float eularY = eular.y() / 180.0f * PI;
	float eularZ = eular.z() / 180.0f * PI;

	float cosP = MyMath::cos( eularX );
	float sinP = MyMath::sin( eularX );

	float cosY = MyMath::cos( eularY );
	float sinY = MyMath::sin( eularY );

	float cosR = MyMath::cos( eularZ );
	float sinR = MyMath::sin( eularZ );

	mat[0][0] = sinR * sinP * sinY + cosR * cosY;
	mat[0][1] = sinR * cosP;
	mat[0][2] = sinR * sinP * cosY - cosR * sinY;
	mat[1][0] = cosR * sinP * sinY - sinR * cosY;
	mat[1][1] = cosR * cosP;
	mat[1][2] = cosR * sinP*cosY + sinR * sinY;
	mat[2][0] = cosP * sinY;
	mat[2][1] = -sinP;
	mat[2][2] = cosP * cosY;
}

RotateMatrix::RotateMatrix( const RotateMatrix &other )
{
	if ( this == &other ) 
		return;

	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 3; ++j )
		{
			mat[i][j] = other.mat[i][j];
		}
	}
}

float RotateMatrix::operator()( int i , int j ) const
{
	return mat[i][j];
}

float& RotateMatrix::operator()( int i , int j )
{
	return mat[i][j];
}

float RotateMatrix::deteminant()
{
	return mat[0][0] * ( mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1] )
		+ mat[0][1] * ( mat[1][2] * mat[2][0] - mat[1][0] * mat[2][2] )
		+ mat[0][2] * ( mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0] );
}

bool RotateMatrix::inverse()
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

RotateMatrix RotateMatrix::operator*( float scale )
{
	RotateMatrix temp( *this );

	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 3; ++j )
		{
			temp.mat[i][j] *= scale;
		}
	}

	return temp;
}

RotateMatrix RotateMatrix::operator/( float scale )
{
	RotateMatrix temp( *this );

	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 3; ++j )
		{
			temp.mat[i][j] /= scale;
		}
	}

	return temp;
}

void RotateMatrix::operator*=( float scale )
{
	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 3; ++j )
		{
			mat[i][j] *= scale;
		}
	}
}

void RotateMatrix::operator/=( float scale )
{
	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 3; ++j )
		{
			mat[i][j] /= scale;
		}
	}
}

Vector3 RotateMatrix::operator*( const Vector3 &v )
{
	return Vector3(
		multiOneRow( *this , v , 0 ) ,
		multiOneRow( *this , v , 1 ) ,
		multiOneRow( *this , v , 2 )
	);
}

RotateMatrix RotateMatrix::operator*( const RotateMatrix &m2 )
{
	RotateMatrix temp;

	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 3; ++j )
		{
			temp.mat[i][j] = multiOneRow( *this , m2 , i , j );
		}
	}

	return temp;
}

void RotateMatrix::operator*=( const RotateMatrix &m2 )
{
	RotateMatrix temp( *this );

	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 3; ++j )
		{
			mat[i][j] = multiOneRow( temp , m2 , i , j );
		}
	}
}

float RotateMatrix::multiOneRow( const RotateMatrix &m1 , const RotateMatrix &m2 , int i , int j )
{
	return m1.mat[i][0] * m2.mat[0][j] + m1.mat[i][1] * m2.mat[1][j] + m1.mat[i][2] * m2.mat[2][j];
}

float RotateMatrix::multiOneRow( const Vector3 &v , const RotateMatrix &m , int j )
{
	return v[0] * m( 0 , j ) + v[1] * m( 1 , j ) + v[2] * m( 2 , j );
}

float RotateMatrix::multiOneRow( const RotateMatrix &m , const Vector3 &v , int i )
{
	return m( i , 0 ) * v[0] + m( i , 1 ) * v[1] + m( i , 2 ) * v[2];
}

////////////////////////////////////友元函数////////////////////////////////////
RotateMatrix operator*( float scale , const RotateMatrix &m )
{
	RotateMatrix temp( m );

	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 3; ++j )
		{
			temp.mat[i][j] *= scale;
		}
	}

	return temp;
}

Vector3 operator*( const Vector3 &v , const RotateMatrix &m ) //行向量 左乘
{
	return Vector3(
		RotateMatrix::multiOneRow( v , m , 0 ) ,
		RotateMatrix::multiOneRow( v , m , 1 ) ,
		RotateMatrix::multiOneRow( v , m , 2 )
	);
}

RotateMatrix operator/( float scale , const RotateMatrix &m )
{
	RotateMatrix temp( m );

	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 3; ++j )
		{
			temp.mat[i][j] /= scale;
		}
	}

	return temp;
}
