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

	virtual const std::vector<CustomVertex>& getVertices() const = 0;
	virtual const std::vector<unsigned int>& getIndices() const = 0;

	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT3 Rotation;
	DirectX::XMFLOAT3 Scale;
	DirectX::XMMATRIX getWorldMatrix() const;

protected:
	std::vector<CustomVertex> vertices;
	std::vector<unsigned int> indices;

	DirectX::XMFLOAT4X4 obj2World;
};

