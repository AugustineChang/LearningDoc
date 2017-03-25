#pragma once
#include "BasicShape.h"


class BasicSquareCone : public BasicShape
{
public:
	BasicSquareCone();
	~BasicSquareCone();

	virtual void UpdateObject( float DeltaTime ) override;
	virtual void createObjectTexture( struct ID3D11Device *device ) override;

protected:

	virtual void createObjectMesh() override;
	float rotSpeed;
};

