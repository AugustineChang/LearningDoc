#pragma once
#include "SimpleTerrain.h"

class WaveTerrain : public SimpleTerrain
{
public:
	WaveTerrain();
	~WaveTerrain();

	virtual void UpdateObject( float DeltaTime ) override;
	void disturb( unsigned int i , unsigned int j , float magnitude );

protected:

	virtual void createObjectMesh() override;
	void UpdateDisturb( float DeltaTime );
	virtual float getHeight( float x , float z , float time ) const override;

private:

	std::vector<CustomVertex> prevVertice;

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

