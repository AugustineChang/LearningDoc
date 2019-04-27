#pragma once

class Vector3;

class Texture
{
public:
	
	virtual Vector3 sample( float u , float v , const Vector3 &worldPos ) const = 0;
};

