#pragma once
#include "BasicShape.h"

class BasicCube :public BasicShape
{
public:
	BasicCube();
	~BasicCube();
	
protected:

	virtual void createObjectMesh() override;
	virtual void createObjectTexture( struct ID3D11Device *device ) override;
};

