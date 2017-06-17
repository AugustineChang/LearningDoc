#pragma once
#include "BasicShape.h"


class BasicSquareCone : public BasicShape
{
public:
	BasicSquareCone();
	~BasicSquareCone();

	virtual void UpdateObject( float DeltaTime , ID3D11DeviceContext *immediateContext ) override;

protected:

	virtual void createObjectMesh() override;
	virtual void createObjectTexture( struct ID3D11Device *device ) override;

	float rotSpeed;
};

