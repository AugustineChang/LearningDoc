#pragma once
#include "BasicShape.h"

class GeoSphere : public BasicShape
{
public:
	GeoSphere();
	~GeoSphere();

	virtual void createObjectMesh() override;

private:

	void doTessllation();

	inline DirectX::XMFLOAT3 float3Mid( const DirectX::XMFLOAT3 &a , const DirectX::XMFLOAT3 &b );
	inline DirectX::XMFLOAT4 float4Mid( const DirectX::XMFLOAT4 &a , const DirectX::XMFLOAT4 &b );

	float radius;
	unsigned int tesselTimes; // tessellation
};

