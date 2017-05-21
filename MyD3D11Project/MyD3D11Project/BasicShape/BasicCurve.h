#pragma once
#include "BasicShape.h"
#include <vector>

class BasicCurve :public BasicShape
{
public:
	BasicCurve();
	~BasicCurve();

protected:

	virtual void createObjectMesh() override;
	virtual void createRenderState( ID3D11Device *device ) override;
	virtual void createObjectTexture( ID3D11Device *device ) override;

	bool getPointOnCurve( float alpha , const std::vector<DirectX::XMVECTOR> &points , DirectX::XMVECTOR &outData );

	std::vector<DirectX::XMFLOAT3> controlPoints;
	int sliceCount;
	float lineWidth;
};

