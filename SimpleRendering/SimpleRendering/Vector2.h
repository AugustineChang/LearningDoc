#pragma once

class Vector2
{
public:

	Vector2()
	{
		vec[0] = 0.0f;
		vec[1] = 0.0f;
	}

	Vector2( float x , float y )
	{
		vec[0] = x;
		vec[1] = y;
	}

	float operator[]( int index ) const
	{ 
		return vec[index];
	}

	float& operator[]( int index )
	{
		return vec[index];
	}

	Vector2 operator+( const Vector2 &v2 ) const;
	Vector2 operator-( const Vector2 &v2 ) const;
	Vector2 operator*( const Vector2 &v2 ) const;
	Vector2 operator/( const Vector2 &v2 ) const;
	
	Vector2 operator*( float scale ) const;
	Vector2 operator/( float scale ) const;
	friend Vector2 operator*( float scale , const Vector2& v );
	friend Vector2 operator/( float scale , const Vector2& v );

	void clamp01();

private:

	float vec[2];
};

Vector2 operator*( float scale , const Vector2& v );
Vector2 operator/( float scale , const Vector2& v );