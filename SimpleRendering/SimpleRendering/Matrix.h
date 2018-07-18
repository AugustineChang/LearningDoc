#pragma once
#include "Vector4.h"

class Matrix
{
public:
	Matrix();
	Matrix( const Vector3& xAxis , const Vector3& yAxis , const Vector3& zAxis ,
		const Vector3& translation = Vector3() );
	Matrix( const Matrix& copy );
	
	float operator()( int i , int j ) const;
	float& operator()( int i , int j );

	void updateMatrix( const Vector3& translation , const Vector3& eular , const Vector3& scale );
	void updateMatrix( const Vector3& translation , const Matrix& rotMat , const Vector3& scale );

	void transpose();
	static void transpose( Matrix& m );
	float deteminant();
	bool inverse();
	static bool inverse( Matrix& m );

	Matrix operator*( float scale );
	Matrix operator/( float scale );
	void operator*=( float scale );
	void operator/=( float scale );
	friend Matrix operator*( float scale , const Matrix &mat );
	friend Matrix operator/( float scale , const Matrix &mat );
	
	Matrix operator*( const Matrix &m2 );
	void operator*=( const Matrix &m2 );
	Vector4 operator*( const Vector4 &v );//列向量 右乘
	friend Vector4 operator*( const Vector4 &v , const Matrix &m );

	friend std::ostream& operator<<( std::ostream& out , const Matrix& m );

public:

	static Matrix translationMatrix( const Vector3& translation );
	static Matrix rotationMatrix( const Vector3& eularAngle );
	static Matrix scalingMatrix( const Vector3& scaling );

private:

	static float multiOneRow( const Matrix &m1 , const Matrix &m2 , int i , int j );
	static float multiOneRow( const Vector4 &v , const Matrix &m , int j );
	static float multiOneRow( const Matrix &m , const Vector4 &v , int i );

	float mat[4][4];
};

Matrix operator*( float scale , const Matrix &m );
Matrix operator/( float scale , const Matrix &m );

Vector4 operator*( const Vector4 &v , const Matrix &m );//行向量 左乘

std::ostream& operator<<( std::ostream& out , const Matrix& m );