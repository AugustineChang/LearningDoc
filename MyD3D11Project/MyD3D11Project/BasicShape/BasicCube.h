#pragma once
#include "BasicShape.h"
#include <vector>

class BasicCube :public BasicShape
{
public:
	BasicCube();
	~BasicCube();
	
protected:

	virtual void UpdateObject( float DeltaTime ) override;
	virtual void createObjectMesh() override;
	virtual void createObjectTexture( struct ID3D11Device *device ) override;

	float timer;
	int curTexture;
	std::vector<ID3D11Resource *> textures;
	std::vector<ID3D11ShaderResourceView *> textureViews;
};

