#pragma once
#include "DirectXApp.h"
#include "Camera.h"
#include "Lights.h"
#include "ShaderEffect.h"
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

	bool isMove;
	POINT lastMousePos;
	float moveSpeed;
	float radius;
	float zoomSpeed;

protected:

	void createObjects();
	void addToGlobalBuffer( std::vector<CustomVertex> &gVBuffer , std::vector<UINT> &gIBuffer , BasicShape &shape );
	void renderObject( const BasicShape &basicObj , UINT indexSize , UINT indexStart , UINT indexBase );

	void createIndexBuffer( const UINT *indices , UINT indexNum , ID3D11Buffer *&indexBuffer );
	template<typename T>
	void createVertexBuffer( const T *vertices , UINT vertexNum , ID3D11Buffer *&vertexBuffer );
	template<typename T>
	void createVertexBuffer2( const T *vertices , UINT vertexNum , ID3D11Buffer *&vertexBuffer );

	Camera camera;
	DirectionalLight dirLight;
	ShaderEffect effect;
	class WaveTerrain *wave;
	class SimpleTerrain *terrain;

	struct ID3D11InputLayout* inputLayout;
	ID3D11Buffer *waveVB;
	ID3D11Buffer *waveIB;
	ID3D11Buffer *otherVB;
	ID3D11Buffer *otherIB;
	struct ID3D11RasterizerState *rasterState;

	//move texture
	DirectX::XMMATRIX moveMatrix;
	DirectX::XMFLOAT2 moveOffset;
};