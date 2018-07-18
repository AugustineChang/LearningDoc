#pragma once
#include "Vector3.h"

class DirectionalLight
{
public:
	DirectionalLight( const Vector3 &eular );

	void setLightDir( const Vector3 &eular );

	Vector3 getLightDir() const
	{
		return worldDir;
	}

private:

	Vector3 worldDir;
};

