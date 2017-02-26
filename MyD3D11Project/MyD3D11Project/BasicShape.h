#pragma once
#include <vector>
#include <DirectXMath.h>

struct CustomVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
};

class BasicShape
{
public:
	
	BasicShape();
	~BasicShape();

	void buildWorldMatrix();

	const std::vector<CustomVertex>& getVertices() const;
	const std::vector<unsigned int>& getIndices() const;

	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT3 Rotation;
	DirectX::XMFLOAT3 Scale;
	DirectX::XMMATRIX getWorldMatrix() const;

	unsigned int indexStart;
	unsigned int indexSize;
	unsigned int indexBase;

protected:

	virtual void createObjectMesh() = 0;

	std::vector<CustomVertex> vertices;
	std::vector<unsigned int> indices;

	DirectX::XMFLOAT4X4 obj2World;
};

