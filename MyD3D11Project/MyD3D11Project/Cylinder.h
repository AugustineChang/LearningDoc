#pragma once
#include "BasicShape.h"

class Cylinder : public BasicShape
{
public:
	Cylinder();
	~Cylinder();

	virtual void createObjectMesh() override;

private:

	void generateCicle( float radius , float height );

	float topRadius;
	float bottomRadius;
	float height;
	unsigned int stackCount;// how many circle in cylinder side
	unsigned int sliceCount;// how many vertices one circle
};

