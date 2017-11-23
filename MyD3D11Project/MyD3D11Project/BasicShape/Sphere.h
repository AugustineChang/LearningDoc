#pragma once
#include "BasicShape.h"

class Sphere : public BasicShape
{
public:
	Sphere();
	~Sphere();

	virtual void UpdateObjectEffect( const Camera *camera ) override;

private:

	virtual void createObjectMesh() override;
	virtual void createEffect( ID3D11Device *device ) override;
	virtual void createObjectTexture( struct ID3D11Device *device ) override;
	virtual void createRenderState( ID3D11Device *device ) override;
	virtual void RenderObject( ID3D11DeviceContext *immediateContext ) override;

	void generateCicle( float vAngle );

	ID3D11Resource *normalTex;
	ID3DX11EffectMatrixVariable *efVP;
	ID3D11ShaderResourceView *normalTexView;
	ID3DX11EffectShaderResourceVariable* efNormalTex;

	float radius;
	unsigned int stackCount;// how many circle in sphere side
	unsigned int sliceCount;// how many vertices one circle
};

