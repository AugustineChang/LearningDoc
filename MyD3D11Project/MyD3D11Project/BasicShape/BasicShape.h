#pragma once
#include <vector>
#include <DirectXMath.h>

struct CustomVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
};

struct CustomMaterial
{
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;// w = SpecPower
};

class BasicShape
{
public:
	
	BasicShape();
	~BasicShape();

	void buildWorldMatrix();
	virtual void UpdateObject( float DeltaTime ){}

	const std::vector<CustomVertex>& getVertices() const;
	const std::vector<unsigned int>& getIndices() const;
	const CustomMaterial& getMaterial() const;

	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT3 Rotation;
	DirectX::XMFLOAT3 Scale;
	DirectX::XMMATRIX getWorldMatrix() const;

	unsigned int indexStart;
	unsigned int indexSize;
	unsigned int indexBase;

protected:

	virtual void createObjectMesh() = 0;
	virtual void computeNormal();

	CustomMaterial material;
	std::vector<CustomVertex> vertices;
	std::vector<unsigned int> indices;

	DirectX::XMFLOAT4X4 obj2World;
};

