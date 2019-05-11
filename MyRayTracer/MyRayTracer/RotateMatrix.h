#pragma once

class Vector3;

class RotateMatrix
{
public:
	RotateMatrix();
	RotateMatrix( const Vector3& eular );
	RotateMatrix( const RotateMatrix &other );
	
	float operator()( int i , int j ) const;
	float& operator()( int i , int j );

	float deteminant();
	bool inverse();

	RotateMatrix operator*( float scale );
	RotateMatrix operator/( float scale );
	void operator*=( float scale );
	void operator/=( float scale );

	Vector3 operator*( const Vector3 &v );//列向量 右乘
	RotateMatrix operator*( const RotateMatrix &m2 );
	void operator*=( const RotateMatrix &m2 );

	friend RotateMatrix operator*( float scale , const RotateMatrix &mat );
	friend RotateMatrix operator/( float scale , const RotateMatrix &mat );
	friend Vector3 operator*( const Vector3 &v , const RotateMatrix &m );// 左乘 行向量

private:

	static float multiOneRow( const RotateMatrix &m1 , const RotateMatrix &m2 , int i , int j );
	static float multiOneRow( const Vector3 &v , const RotateMatrix &m , int j );
	static float multiOneRow( const RotateMatrix &m , const Vector3 &v , int i );

	float mat[3][3];
};

