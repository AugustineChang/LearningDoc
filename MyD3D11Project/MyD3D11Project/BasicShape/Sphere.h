#pragma once
#include "BasicShape.h"

class Sphere : public BasicShape
{
public:
	Sphere();
	~Sphere();

	virtual void createObjectMesh() override;
	virtual void createObjectTexture( struct ID3D11Device *device ) override;

private:

	void generateCicle( float vAngle );

	float radius;
	unsigned int stackCount;// how many circle in sphere side
	unsigned int sliceCount;// how many vertices one circle
};

