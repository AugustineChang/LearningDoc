#pragma once
#include "SimpleTerrain.h"

class WaveTerrain : public SimpleTerrain
{
public:
	WaveTerrain();
	~WaveTerrain();

	virtual void UpdateObject( float DeltaTime , ID3D11DeviceContext *immediateContext ) override;
	void disturb( unsigned int i , unsigned int j , float magnitude );

protected:

	virtual void computeNormal() override;
	virtual void createObjectMesh() override;
	virtual void createObjectTexture( struct ID3D11Device *device ) override;
	virtual void createBlendState( ID3D11Device *device ) override;
	virtual void computeBoundingBox() override;

	void UpdateDisturb( float DeltaTime );
	virtual float getHeight( float x , float z ) const override;

private:

	std::vector<BaseVertex> prevVertice;

	float timer;
	float disturbTimer;
	float dx;
	float timeStep;
	float damping;
	float speed;

	float K1;
	float K2;
	float K3;

};

