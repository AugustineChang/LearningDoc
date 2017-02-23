#pragma once
#include "DirectXApp.h"
#include <DirectXMath.h>

struct CustomVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
};

class MyFirstDX11App : public DirectXApp
{
public:
	MyFirstDX11App( HINSTANCE hinstance , int show );
	~MyFirstDX11App();

	virtual void UpdateScene( float deltaTime ) override;
	virtual void DrawScene() override;

private:

	void createInputLayout();
	void createVertexBuffer( const CustomVertex *vertices , UINT vertexNum );
	void createIndexBuffer( const UINT *indices , UINT indexNum );
	void setRenderState();
	void createEffect();

	void createCube();
};