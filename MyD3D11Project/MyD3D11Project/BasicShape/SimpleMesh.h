#pragma once
#include "BasicShape.h"
class SimpleMesh : public BasicShape
{
public:
	SimpleMesh();
	~SimpleMesh();

private:

	virtual void createObjectMesh() override;

	virtual void createEffect( ID3D11Device *device ) override;
	virtual void createObjectTexture( ID3D11Device *device ) override;
	virtual void UpdateObjectEffect( const Camera *camera ) override;

	std::string filePath;
	ID3D11Resource *reflTexture;
	ID3D11ShaderResourceView *reflTextureView;
	ID3DX11EffectShaderResourceVariable* efReflTexture;
};

