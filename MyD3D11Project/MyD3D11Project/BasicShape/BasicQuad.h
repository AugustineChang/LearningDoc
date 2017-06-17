#pragma once
#include "BasicShape.h"
#include <vector>

struct TessVertex;

class BasicQuad :public BasicShape
{
public:
	BasicQuad();
	~BasicQuad();
	
	virtual void UpdateObjectEffect( const Camera *camera ) override;
	virtual void UpdateDirectionalLight( const DirectionalLight *dirLight , int lightNum ) override {}
	virtual void UpdatePointLight( const PointLight *pointLight , int lightNum ) override {}
	virtual void UpdateSpotLight( const SpotLight *spotLight , int lightNum ) override {}
	virtual void RenderObject( ID3D11DeviceContext *immediateContext ) override;

	const std::vector<TessVertex>& getVertices() const { return tessVertices; }

protected:

	virtual void createObjectMesh() override;
	virtual void createInputLayout( ID3D11Device *device );
	virtual void createEffect( ID3D11Device *device ) override;
	virtual void createRenderState( ID3D11Device *device ) override;
	virtual void createObjectTexture( ID3D11Device *device ) override;

	float width;
	float height;
	std::vector<TessVertex> tessVertices;
};

