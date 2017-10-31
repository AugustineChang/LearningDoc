#pragma once
#include "GeoSphere.h"

class SkySphere : public GeoSphere
{
public:
	SkySphere();
	~SkySphere();

	virtual void UpdateDirectionalLight( const DirectionalLight *dirLight , int lightNum ) override {}
	virtual void UpdatePointLight( const PointLight *pointLight , int lightNum ) override {}
	virtual void UpdateSpotLight( const SpotLight *spotLight , int lightNum ) override {}

protected:

	virtual void createEffect( ID3D11Device *device ) override;
	virtual void createRenderState( ID3D11Device *device );
	virtual void createObjectTexture( ID3D11Device *device ) override;
	virtual void UpdateObjectEffect( const Camera *camera ) override;
};

