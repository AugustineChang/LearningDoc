#pragma once
#include <DirectXMath.h>

class ShaderEffect
{
public:
	ShaderEffect();
	~ShaderEffect();

	void createEffectAtRuntime( struct ID3D11Device *device );
	void createEffectAtBuildtime( struct ID3D11Device *device );

	void UpdateSceneEffect( class Camera *camera , struct DirectionalLight *dirLigiht ,
		struct PointLight *pointLigiht , struct SpotLight *spotLigiht );
	void UpdateObjectEffect( struct DirectX::XMMATRIX &maxtrixWVP ,
		DirectX::XMMATRIX &toWorld , DirectX::XMMATRIX &normToWorld ,
		DirectX::XMMATRIX &texMatrix , const class BasicShape *obj );

private:

	struct ID3DX11Effect *effect;
	struct ID3DX11EffectTechnique *effectTech;
	struct ID3DX11EffectMatrixVariable *efWVP;
	struct ID3DX11EffectMatrixVariable *efWorld;
	struct ID3DX11EffectMatrixVariable *efWorldNorm;
	struct ID3DX11EffectMatrixVariable *efTexTrans;

	struct ID3DX11EffectVariable *efMaterial;
	struct ID3DX11EffectShaderResourceVariable* efTexture;

	struct ID3DX11EffectVariable *efDirLight;
	struct ID3DX11EffectVariable *efPointLight;
	struct ID3DX11EffectVariable *efSpotLight;
	struct ID3DX11EffectVectorVariable *efCameraPos;

public:

	ID3DX11EffectTechnique *getEffectTech();

};

