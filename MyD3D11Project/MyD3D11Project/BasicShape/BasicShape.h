#pragma once
#include <vector>
#include <DirectXMath.h>

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

class BasicShape
{
public:
	
	BasicShape();
	~BasicShape();

	void buildWorldMatrix();
	void InitShape( struct ID3D11Device *device );
	virtual void UpdateObject( float DeltaTime ){}

	const std::vector<CustomVertex>& getVertices() const;
	const std::vector<unsigned int>& getIndices() const;
	const CustomMaterial& getMaterial() const;
	ID3D11ShaderResourceView* getTexture() const;
	ID3D11ShaderResourceView* getAlphaTexture() const;
	ID3D11BlendState* getBlendState() const;

	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT3 Rotation;
	DirectX::XMFLOAT3 Scale;
	DirectX::XMMATRIX getWorldMatrix() const;

	unsigned int indexStart;
	unsigned int indexSize;
	unsigned int indexBase;

protected:

	virtual void createBlendState( ID3D11Device *device ) {}
	virtual void createObjectTexture( ID3D11Device *device ) {}
	virtual void createObjectMesh() = 0;
	virtual void computeNormal();

	CustomMaterial material;
	std::vector<CustomVertex> vertices;
	std::vector<unsigned int> indices;

	ID3D11Resource *texture;
	ID3D11ShaderResourceView *textureView;
	ID3D11Resource *alphaTexture;
	ID3D11ShaderResourceView *alphaTextureView;
	ID3D11BlendState *blendState;

	DirectX::XMFLOAT4X4 obj2World;
};

