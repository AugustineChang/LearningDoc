#pragma once
#include "BasicShape.h"
#include <vector>

class BasicCube :public BasicShape
{
public:
	BasicCube();
	~BasicCube();
	
	virtual void UpdateObject( float DeltaTime ) override;

protected:

	virtual void createObjectMesh() override;
	virtual void createBlendState( struct ID3D11Device *device ) override;
	virtual void createObjectTexture( ID3D11Device *device ) override;

	float timer;
	int curTexture;
	std::vector<ID3D11Resource *> textures;
	std::vector<ID3D11ShaderResourceView *> textureViews;
};

