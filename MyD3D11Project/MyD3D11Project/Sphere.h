#pragma once
#include "BasicShape.h"

class Sphere : public BasicShape
{
public:
	Sphere();
	~Sphere();

	virtual void createObjectMesh() override;
	void generateCicle( float vAngle );

private:

	float radius;
	unsigned int stackCount;// how many circle in sphere side
	unsigned int sliceCount;// how many vertices one circle
};

