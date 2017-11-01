#pragma once
#include "BasicShape.h"

class GeoSphere : public BasicShape
{
public:
	GeoSphere();
	~GeoSphere();

	virtual void UpdateObjectEffect( const Camera *camera ) override;

private:

	virtual void createObjectMesh() override;
	virtual void createEffect( ID3D11Device *device ) override;
	virtual void createObjectTexture( struct ID3D11Device *device ) override;

	void doTessllation();

	inline DirectX::XMFLOAT3 float3Mid( const DirectX::XMFLOAT3 &a , const DirectX::XMFLOAT3 &b );
	inline DirectX::XMFLOAT4 float4Mid( const DirectX::XMFLOAT4 &a , const DirectX::XMFLOAT4 &b );

	ID3D11Resource *normalTex;
	ID3D11ShaderResourceView *normalTexView;
	ID3DX11EffectShaderResourceVariable* efNormalTex;

	float radius;
	unsigned int tesselTimes; // tessellation
};

