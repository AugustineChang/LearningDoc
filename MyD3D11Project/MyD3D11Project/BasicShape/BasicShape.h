#pragma once
#include <vector>
#include <DirectXMath.h>
#include "../DirectXApp/ShaderEffect.h"

struct CustomVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexCoord;
};

struct CustomMaterial
{
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;// w = SpecPower
};

struct ID3D11Resource;
struct ID3D11ShaderResourceView;
struct ID3D11BlendState;
struct ID3D11InputLayout;
struct ID3D11RasterizerState;
struct ID3D11Device;
struct ID3D11DeviceContext;

struct ID3DX11EffectVariable;
struct ID3DX11EffectMatrixVariable;
struct ID3DX11EffectVectorVariable;
struct ID3DX11EffectScalarVariable;
struct ID3DX11EffectShaderResourceVariable;

class Camera;
struct DirectionalLight;

class BasicShape
{
public:
	
	BasicShape();
	BasicShape( std::string shader );
	~BasicShape();
	
	void InitShape( ID3D11Device *device );
	virtual void UpdateObject( float DeltaTime ){}
	virtual void UpdateObjectEffect( const Camera *camera , const DirectionalLight *dirLight );
	virtual void RenderObject( ID3D11DeviceContext *immediateContext );

	const std::vector<CustomVertex>& getVertices() const;
	const std::vector<unsigned int>& getIndices() const;

	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT3 Rotation;
	DirectX::XMFLOAT3 Scale;

	unsigned int indexStart;
	unsigned int indexSize;
	unsigned int indexBase;

protected:

	void initDirectMath();
	void buildWorldMatrix();

	virtual void createEffect( ID3D11Device *device );
	virtual void createInputLayout( ID3D11Device *device );
	virtual void createRenderState( ID3D11Device *device );
	virtual void createBlendState( ID3D11Device *device ) {}
	virtual void createObjectTexture( ID3D11Device *device ) {}
	virtual void createObjectMesh() = 0;
	virtual void computeNormal();

	ShaderEffect effect;
	std::string techName;
	ID3DX11EffectMatrixVariable *efWVP;
	ID3DX11EffectMatrixVariable *efWorld;
	ID3DX11EffectMatrixVariable *efWorldNorm;
	ID3DX11EffectMatrixVariable *efTexTrans;
	ID3DX11EffectVariable *efMaterial;
	ID3DX11EffectShaderResourceVariable* efTexture;
	ID3DX11EffectVariable *efDirLight;
	ID3DX11EffectVectorVariable *efCameraPos;
	
	//Fog
	bool isEnableFog;
	float fogStart;
	float fogDistance;
	DirectX::XMFLOAT4 fogColor;
	ID3DX11EffectScalarVariable *efFogStart;
	ID3DX11EffectScalarVariable *efFogDistance;
	ID3DX11EffectVectorVariable *efFogColor;

	CustomMaterial material;
	std::vector<CustomVertex> vertices;
	std::vector<unsigned int> indices;

	ID3D11Resource *texture;
	ID3D11ShaderResourceView *textureView;
	ID3D11InputLayout* inputLayout;
	ID3D11RasterizerState *rasterState;
	ID3D11BlendState *blendState;

	DirectX::XMFLOAT4X4 obj2World;
};

