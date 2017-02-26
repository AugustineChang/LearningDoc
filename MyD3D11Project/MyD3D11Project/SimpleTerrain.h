#pragma once
#include "BasicShape.h"
#include "SimpleMath.h"

class SimpleTerrain : public BasicShape
{
public:
	SimpleTerrain();
	~SimpleTerrain();

protected:

	virtual void createObjectMesh() override;

private:

	Point<unsigned int> verticesDim;// GridVertices m x n
	Point<float> terrainSize;// Terrain real width and length
};

