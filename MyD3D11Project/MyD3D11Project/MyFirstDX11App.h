#pragma once
#include "DirectXApp.h"

class MyFirstDX11App : public DirectXApp
{
public:
	MyFirstDX11App( HINSTANCE hinstance , int show );
	~MyFirstDX11App();

	virtual void UpdateScene( float deltaTime ) override;
	virtual void DrawScene() override;
};

