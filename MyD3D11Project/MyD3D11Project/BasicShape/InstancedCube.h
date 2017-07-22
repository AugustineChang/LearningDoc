#pragma once
#include "BasicShape.h"
#include <vector>

class InstancedCube :public BasicShape
{
public:
	InstancedCube();
	~InstancedCube();
	
	virtual void UpdateObject( float DeltaTime , ID3D11DeviceContext *immediateContext ) override;
	virtual void RenderObject( ID3D11DeviceContext *immediateContext ) override;
	virtual void UpdateObjectEffect( const Camera *camera ) override;
	virtual void doFrustumCull( const Camera *camera ) override;

protected:

	virtual void InitShape( ID3D11Device *device ) override;
	virtual void createObjectMesh() override;
	virtual void createInputLayout( ID3D11Device *device ) override;
	virtual void createRenderState( ID3D11Device *device ) override;
	virtual void createObjectTexture( ID3D11Device *device ) override;

private:

	bool doInstancedFrustumCull( const InstanceData *data );
	void createInstanceData( ID3D11Device *device );
	void updateOneInstanceData( InstanceData *data , const DirectX::XMFLOAT3 &center , const DirectX::XMFLOAT3 &delta ,
		int xIndex , int yIndex , int zIndex );

	const class Camera *cachedCamera;
	int edgeNum;
	float edgeLength;
	size_t realDrawNum;
	std::vector<InstanceData> instanceDataList;
};

