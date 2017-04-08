#pragma once
#include "BasicShape.h"
#include <vector>

class RotateFlame :public BasicShape
{
public:
	RotateFlame();
	~RotateFlame();
	
	virtual void UpdateObject( float DeltaTime ) override;
	virtual void UpdateDirectionalLight( const DirectionalLight *dirLight , int lightNum ) override;
	virtual void UpdatePointLight( const PointLight *pointLight , int lightNum ) override {}
	virtual void UpdateSpotLight( const SpotLight *spotLight , int lightNum ) override {}

protected:

	virtual void createObjectMesh() override;
	virtual void createEffect( ID3D11Device *device ) override;
	virtual void createObjectTexture( ID3D11Device *device ) override;
	virtual void UpdateObjectEffect( const Camera *camera ) override;

	ID3D11Resource *alphaTexture;
	ID3D11ShaderResourceView *alphaTextureView;
	ID3DX11EffectShaderResourceVariable* efAlphaTexture;

	DirectX::XMMATRIX rotMatrix;
	float rotAngle;
};

