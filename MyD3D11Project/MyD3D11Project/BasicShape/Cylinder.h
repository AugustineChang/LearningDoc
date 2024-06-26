#pragma once
#include "BasicShape.h"

class Cylinder : public BasicShape
{
public:
	Cylinder();
	~Cylinder();

	virtual void UpdateObjectEffect( const Camera *camera ) override;

private:

	virtual void createObjectMesh() override;
	virtual void createEffect( ID3D11Device *device ) override;
	virtual void createObjectTexture( struct ID3D11Device *device ) override;
	void generateCicle( float radius , float height );

	ID3D11Resource *normalTex;
	ID3D11ShaderResourceView *normalTexView;
	ID3DX11EffectShaderResourceVariable* efNormalTex;

	float topRadius;
	float bottomRadius;
	float height;
	unsigned int stackCount;// how many circle in cylinder side
	unsigned int sliceCount;// how many vertices one circle
};

