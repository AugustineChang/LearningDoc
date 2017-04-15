#pragma once
#include "BasicShape.h"

struct ID3D11Buffer;

class Billboard : public BasicShape
{
public:
	Billboard();
	~Billboard();

	virtual void InitShape( ID3D11Device *device ) override;
	virtual void UpdateDirectionalLight( const DirectionalLight *dirLight , int lightNum ) override {}
	virtual void UpdatePointLight( const PointLight *pointLight , int lightNum ) override {}
	virtual void UpdateSpotLight( const SpotLight *spotLight , int lightNum ) override {}

	virtual void UpdateObjectEffect( const Camera *camera ) override;
	virtual void RenderObject( ID3D11DeviceContext *immediateContext ) override;

private:

	virtual void createEffect( ID3D11Device *device ) override;
	virtual void createInputLayout( ID3D11Device *device ) override;
	virtual void createObjectTexture( ID3D11Device *device ) override;
	virtual void createObjectMesh() override {}

	void createBuffers( ID3D11Device *device );

	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer;
	ID3DX11EffectVectorVariable *efCameraUp;

};

