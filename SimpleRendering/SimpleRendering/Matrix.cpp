#include "Matrix.h"
#include "Matrix3x3.h"
#include "SimpleMath.h"
#include <math.h>
#include <iomanip>

Matrix::Matrix()
{
	for ( int i = 0; i < 4; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			mat[i][j] = 0.0f;
		}
	}
}

Matrix::Matrix( const Matrix& copy )
{
	if ( this == &copy )return;

	for ( int i = 0; i < 4; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			mat[i][j] = copy.mat[i][j];
		}
	}
}

Matrix::Matrix( const Vector3& xAxis , const Vector3& yAxis , const Vector3& zAxis , 
	const Vector3& translation /*= Vector3() */ )
{
	mat[0][0] = xAxis[0];
	mat[0][1] = xAxis[1];
	mat[0][2] = xAxis[2];
	mat[0][3] = 0.0f;

	mat[1][0] = yAxis[0];
	mat[1][1] = yAxis[1];
	mat[1][2] = yAxis[2];
	mat[1][3] = 0.0f;

	mat[2][0] = zAxis[0];
	mat[2][1] = zAxis[1];
	mat[2][2] = zAxis[2];
	mat[2][3] = 0.0f;

	mat[3][0] = translation[0];
	mat[3][1] = translation[1];
	mat[3][2] = translation[2];
	mat[3][3] = 1.0f;
}

float Matrix::operator()( int i , int j ) const
{
	return mat[i][j];
}

float& Matrix::operator()( int i , int j )
{
	return mat[i][j];
}

void Matrix::updateMatrix( const Vector3& translation , const Vector3& eular , const Vector3& scale )
{
	Matrix tMat = translationMatrix( translation );
	Matrix rMat = rotationMatrix( eular );
	Matrix sMat = scalingMatrix( scale );

	Matrix resultMat = sMat * rMat * tMat;
	for ( int i = 0; i < 4; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			mat[i][j] = resultMat.mat[i][j];
		}
	}
}

void Matrix::updateMatrix( const Vector3& translation , const Matrix& rotMat , const Vector3& scale )
{
	Matrix tMat = translationMatrix( translation );
	Matrix sMat = scalingMatrix( scale );

	Matrix resultMat = sMat * rotMat * tMat;
	for ( int i = 0; i < 4; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			mat[i][j] = resultMat.mat[i][j];
		}
	}
}

void Matrix::transpose()
{
	Matrix::transpose( *this );
}

void Matrix::transpose( Matrix& m )
{
	Matrix temp( m );

	for ( int i = 0; i < 4; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			m.mat[i][j] = temp.mat[j][i];
		}
	}
}

float Matrix::deteminant()
{
	float detM11 = Matrix3x3( *this , 0 , 0 ).deteminant();
	float detM12 = Matrix3x3( *this , 0 , 1 ).deteminant();
	float detM13 = Matrix3x3( *this , 0 , 2 ).deteminant();
	float detM14 = Matrix3x3( *this , 0 , 3 ).deteminant();

	return mat[0][0] * detM11 - mat[0][1] * detM12 + mat[0][2] * detM13 - mat[0][3] * detM14;
}

bool Matrix::inverse()
{
	return Matrix::inverse( *this );
}

bool Matrix::inverse( Matrix& m )
{
	//先求左上3x3
	Matrix3x3 c44 = Matrix3x3( m , 3 , 3 );
	if ( !c44.inverse() ) return false;

	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 3; ++j )
		{
			m.mat[i][j] = c44( i , j );
		}
	}

	//补齐右侧 4×1
	m.mat[0][3] = 0.0f;
	m.mat[1][3] = 0.0f;
	m.mat[2][3] = 0.0f;
	m.mat[3][3] = 1.0f;

	//再求下方1x3
	Vector4 temp( m.mat[3][0] , m.mat[3][1] , m.mat[3][2] , 0.0f );
	temp = temp * ( m );

	m.mat[3][0] = -temp[0];
	m.mat[3][1] = -temp[1];
	m.mat[3][2] = -temp[2];
	return true;
}

Matrix Matrix::operator*( float scale )
{
	Matrix temp( *this );

	for ( int i = 0; i < 4; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			temp.mat[i][j] *= scale;
		}
	}

	return temp;
}

Matrix Matrix::operator*( const Matrix &m2 )
{
	Matrix temp;

	for ( int i = 0; i < 4; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			temp.mat[i][j] = multiOneRow( *this , m2 , i , j );
		}
	}

	return temp;
}

void Matrix::operator*=( const Matrix &m2 )
{
	Matrix temp( *this );

	for ( int i = 0; i < 4; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			mat[i][j] = multiOneRow( temp , m2 , i , j );
		}
	}
}

Vector4 Matrix::operator*( const Vector4 &v )//列向量 右乘
{
	return Vector4(
		multiOneRow( *this , v , 0 ) ,
		multiOneRow( *this , v , 1 ) ,
		multiOneRow( *this , v , 2 ) ,
		multiOneRow( *this , v , 3 )
	);
}

Matrix Matrix::operator/( float scale )
{
	Matrix temp( *this );

	for ( int i = 0; i < 4; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			temp.mat[i][j] /= scale;
		}
	}

	return temp;
}

void Matrix::operator*=( float scale )
{
	for ( int i = 0; i < 4; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			mat[i][j] *= scale;
		}
	}
}


void Matrix::operator/=( float scale )
{
	for ( int i = 0; i < 4; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			mat[i][j] /= scale;
		}
	}
}
////////////////////////////////////私有函数////////////////////////////////////
Matrix Matrix::translationMatrix( const Vector3& translation )
{
	Matrix temp;
	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			if ( i == j )temp.mat[i][j] = 1.0f;
			else temp.mat[i][j] = 0.0f;
		}
	}

	temp.mat[3][0] = translation.x();
	temp.mat[3][1] = translation.y();
	temp.mat[3][2] = translation.z();
	temp.mat[3][3] = 1.0f;
	return temp;
}

Matrix Matrix::rotationMatrix( const Vector3& eularAngle )
{
	//旋转顺序 roll->pitch->yaw

	//roll x轴
	// 1    0    0
	// 0    cosR sinR
	// 0   -sinR cosR

	//pitch y轴
	// cosP 0   -sinP
	// 0    1    0
	// sinP 0    cosP

	//yaw z轴
	// cosY sinY 0
	//-sinY cosY 0
	// 0    0    1


	//结果
	//cosPcosY               cosPsinY               -sinP
	//sinRsinPcosY-cosRsinY  sinRsinPsinY+cosRcosY  sinRcosP
	//cosRsinPcosY+sinRsinY  cosRsinPsinY-sinRcosY  cosRcosP

	float eularX = eularAngle.x() / 180.0f * PI;
	float eularY = eularAngle.y() / 180.0f * PI;
	float eularZ = eularAngle.z() / 180.0f * PI;

	float cosR = cosf( eularX );
	float sinR = sinf( eularX );

	float cosP = cosf( eularY );
	float sinP = sinf( eularY );

	float cosY = cosf( eularZ );
	float sinY = sinf( eularZ );

	Matrix temp;
	temp.mat[0][0] = cosP*cosY;
	temp.mat[0][1] = cosP*sinY;
	temp.mat[0][2] = -sinP;
	temp.mat[1][0] = sinR*sinP*cosY - cosR*sinY;
	temp.mat[1][1] = sinR*sinP*sinY + cosR*cosY;
	temp.mat[1][2] = sinR*cosP;
	temp.mat[2][0] = cosR*sinP*cosY + sinR*sinY;
	temp.mat[2][1] = cosR*sinP*sinY - sinR*cosY;
	temp.mat[2][2] = cosR*cosP;

	temp.mat[3][0] = 0.0f;
	temp.mat[3][1] = 0.0f;
	temp.mat[3][2] = 0.0f;
	temp.mat[3][3] = 1.0f;

	return temp;
}

Matrix Matrix::scalingMatrix( const Vector3& scaling )
{
	Matrix temp;
	for ( int i = 0; i < 3; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			if ( i == j )temp.mat[i][j] = scaling[i];
			else temp.mat[i][j] = 0.0f;
		}
	}

	temp.mat[3][0] = 0.0f;
	temp.mat[3][1] = 0.0f;
	temp.mat[3][2] = 0.0f;
	temp.mat[3][3] = 1.0f;
	return temp;
}

float Matrix::multiOneRow( const Matrix &m1 , const Matrix &m2 , int i , int j )
{
	return m1.mat[i][0] * m2.mat[0][j] + m1.mat[i][1] * m2.mat[1][j] + m1.mat[i][2] * m2.mat[2][j] +
		m1.mat[i][3] * m2.mat[3][j];
}

float Matrix::multiOneRow( const Vector4 &v , const Matrix &m , int j )
{
	return v[0] * m( 0 , j ) + v[1] * m( 1 , j ) + v[2] * m( 2 , j ) + v[3] * m( 3 , j );
}

float Matrix::multiOneRow( const Matrix &m , const Vector4 &v , int i )
{
	return m( i , 0 ) * v[0] + m( i , 1 ) * v[1] + m( i , 2 ) * v[2] + m( i , 3 ) * v[3];
}

////////////////////////////////////友元函数////////////////////////////////////
Matrix operator*( float scale , const Matrix &m )
{
	Matrix temp( m );

	for ( int i = 0; i < 4; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			temp.mat[i][j] *= scale;
		}
	}

	return temp;
}

Vector4 operator*( const Vector4 &v , const Matrix &m ) //行向量 左乘
{
	return Vector4(
		Matrix::multiOneRow( v , m , 0 ) ,
		Matrix::multiOneRow( v , m , 1 ) ,
		Matrix::multiOneRow( v , m , 2 ) ,
		Matrix::multiOneRow( v , m , 3 )
	);
}

Matrix operator/( float scale , const Matrix &m )
{
	Matrix temp( m );

	for ( int i = 0; i < 4; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			temp.mat[i][j] /= scale;
		}
	}

	return temp;
}

std::ostream& operator<<( std::ostream& out , const Matrix& m )
{
	out << std::setprecision( 3 );

	for ( int i = 0; i < 4; ++i )
	{
		for ( int j = 0; j < 4; ++j )
		{
			out << std::setfill( ' ' ) << std::setw( 10 ) << m.mat[i][j];
		}
		out << '\n';
	}

	return out;
}
