#pragma once
#include "BasicShape.h"
#include <vector>

class BlendFlame :public BasicShape
{
public:
	BlendFlame();
	~BlendFlame();
	
	virtual void UpdateObject( float DeltaTime ) override;
	virtual void UpdateDirectionalLight( const DirectionalLight *dirLight , int lightNum ) override;
	virtual void UpdatePointLight( const PointLight *pointLight , int lightNum ) override {}
	virtual void UpdateSpotLight( const SpotLight *spotLight , int lightNum ) override {}

protected:

	virtual void createObjectMesh() override;
	virtual void createBlendState( struct ID3D11Device *device ) override;
	virtual void createObjectTexture( ID3D11Device *device ) override;

	float timer;
	int curTexture;
	std::vector<ID3D11Resource *> textures;
	std::vector<ID3D11ShaderResourceView *> textureViews;
};

