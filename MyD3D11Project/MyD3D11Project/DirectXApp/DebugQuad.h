#pragma once
#include "../BasicShape/BasicShape.h"

struct ID3D11Buffer;
struct ID3DX11EffectShaderResourceVariable;

class DebugQuad : public BasicShape
{
public:
	DebugQuad();
	~DebugQuad();
	
	virtual void UpdateDirectionalLight( const DirectionalLight *dirLight , int lightNum ) override {}
	virtual void UpdatePointLight( const PointLight *pointLight , int lightNum ) override {}
	virtual void UpdateSpotLight( const SpotLight *spotLight , int lightNum ) override {}

	virtual void UpdateObjectEffect( const Camera *camera ) override {}
	virtual void RenderObject( ID3D11DeviceContext *immediateContext ) override;

	void UpdateDebugTexture( ID3D11ShaderResourceView *texView );

private:

	virtual void createEffect( ID3D11Device *device ) override;
	virtual void createInputLayout( ID3D11Device *device ) override {}
	virtual void createObjectMesh() override {}

	ID3DX11EffectShaderResourceVariable* efDepthBuf;
};