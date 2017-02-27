#pragma once
#include "BasicShape.h"
#include "../Utilities/SimpleMath.h"

class SimpleTerrain : public BasicShape
{
public:
	SimpleTerrain();
	SimpleTerrain( const Point<unsigned int> &vert , const Point<float> &size );
	~SimpleTerrain();

protected:

	virtual void createObjectMesh() override;
	virtual void createBasicPlane();
	virtual float getHeight( float x , float z , float time ) const;

	Point<unsigned int> verticesDim;// GridVertices m x n
	Point<float> terrainSize;// Terrain real width and length
};

