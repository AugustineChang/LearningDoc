#pragma once
#include "DirectXApp.h"
#include "Camera.h"
#include <DirectXMath.h>
#include <vector>

struct CustomVertex;
struct ID3D11Buffer;

class WaveScene : public DirectXApp
{
public:
	WaveScene( HINSTANCE hinstance , int show );
	WaveScene( const WaveScene &copy ) = delete;
	WaveScene& operator=( const WaveScene &copy ) = delete;
	~WaveScene();

	virtual bool InitDirectApp() override;
	virtual void UpdateScene( float deltaTime ) override;
	virtual void DrawScene() override;

	virtual void OnResize() override;
	virtual void OnMouseDown( WPARAM btnState , int x , int y ) override;
	virtual void OnMouseMove( WPARAM btnState , int x , int y ) override;
	virtual void OnMouseUp( WPARAM btnState , int x , int y ) override;
	virtual void OnMouseWheel( int zDelta ) override;

private:

	void createInputLayout();
	void createRenderState();
	void createEffectAtRuntime();
	void createEffectAtBuildtime();

	bool isMove;
	POINT lastMousePos;
	float moveSpeed;
	float radius;
	float zoomSpeed;

protected:

	void createObjects();
	void createIndexBuffer( const UINT *indices , UINT indexNum , ID3D11Buffer *&indexBuffer );

	template<typename T>
	void createVertexBuffer( const T *vertices , UINT vertexNum , ID3D11Buffer *&vertexBuffer );

	Camera camera;
	class WaveTerrain *wave;

	struct ID3D11InputLayout* inputLayout;
	ID3D11Buffer *waveVB[2];
	ID3D11Buffer *waveIB;
	struct ID3D11RasterizerState *rasterState;
	struct ID3DX11Effect *effect;
	struct ID3DX11EffectTechnique *effectTech;
	struct ID3DX11EffectMatrixVariable *effectWVP;

};