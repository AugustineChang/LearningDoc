#pragma once
#include "BasicShape.h"
#include <vector>

class RotateFlame :public BasicShape
{
public:
	RotateFlame();
	~RotateFlame();
	
	virtual void UpdateObject( float DeltaTime ) override;

protected:

	virtual void createObjectMesh() override;
	virtual void createEffect( ID3D11Device *device ) override;
	virtual void createObjectTexture( ID3D11Device *device ) override;
	virtual void UpdateObjectEffect( const Camera *camera , const DirectionalLight *dirLight ) override;

	ID3D11Resource *alphaTexture;
	ID3D11ShaderResourceView *alphaTextureView;
	ID3DX11EffectShaderResourceVariable* efAlphaTexture;

	DirectX::XMMATRIX rotMatrix;
	float rotAngle;
};

