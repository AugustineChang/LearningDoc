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

	virtual void createObjectTexture( struct ID3D11Device *device ) override;
	virtual void createObjectMesh() override;
	virtual void createBasicPlane();
	virtual void computeBoundingBox() override;
	virtual float getHeight( float x , float z ) const;

	virtual void createRenderState( ID3D11Device *device ) override;

	Point<unsigned int> verticesDim;// GridVertices m x n
	Point<float> terrainSize;// Terrain real width and length
};

