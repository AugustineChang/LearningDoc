#pragma once
#include "DirectXApp.h"
#include "Camera.h"
#include "BasicCube.h"
#include "BasicSquareCone.h"
#include <DirectXMath.h>

class SimpleScene : public DirectXApp
{
public:
	SimpleScene( HINSTANCE hinstance , int show );
	~SimpleScene();

	virtual bool InitDirectApp() override;
	virtual void UpdateScene( float deltaTime ) override;
	virtual void DrawScene() override;

	virtual void OnResize() override;
	virtual void OnMouseDown( WPARAM btnState , int x , int y ) override;
	virtual void OnMouseMove( WPARAM btnState , int x , int y ) override;
	virtual void OnMouseUp( WPARAM btnState , int x , int y ) override;

private:

	void createInputLayout();
	void createRenderState();
	void createEffectAtRuntime();
	void createEffectAtBuildtime();

	void createObjects();
	void createVertexBuffer( const CustomVertex *vertices , UINT vertexNum );
	void createIndexBuffer( const UINT *indices , UINT indexNum );
	void renderObject( const BasicShape &basicObj , UINT indexSize , UINT indexStart , UINT indexBase );
	
	Camera camera;
	BasicCube cube;
	BasicSquareCone squareCone;

	bool isMove;
	POINT lastMousePos;
	float moveSpeed;

	struct ID3D11InputLayout* inputLayout;
	struct ID3D11Buffer *vertexBuffer;
	struct ID3D11Buffer *indexBuffer;
	struct ID3D11RasterizerState *rasterState;
	struct ID3DX11Effect *effect;
	struct ID3DX11EffectTechnique *effectTech;
	struct ID3DX11EffectMatrixVariable *effectWVP;
};