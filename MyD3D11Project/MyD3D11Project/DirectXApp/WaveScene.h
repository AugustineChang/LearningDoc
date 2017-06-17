#pragma once
#include "DirectXApp.h"
#include "Camera.h"
#include "Lights.h"
#include <DirectXMath.h>
#include <vector>

struct BaseVertex;
struct ID3D11Buffer;
class BasicShape;

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

	bool isMove;
	POINT lastMousePos;
	float rotSpeed;
	float moveSpeed;

protected:

	void createObjects();
	void addToGlobalBuffer( std::vector<BaseVertex> &gVBuffer , std::vector<UINT> &gIBuffer , BasicShape &shape );
	
	void createIndexBuffer( const UINT *indices , UINT indexNum , ID3D11Buffer *&indexBuffer );
	template<typename T>
	void createVertexBuffer( const T *vertices , UINT vertexNum , ID3D11Buffer *&vertexBuffer );
	template<typename T>
	void createVertexBuffer2( const T *vertices , UINT vertexNum , ID3D11Buffer *&vertexBuffer );

	Camera camera;
	DirectionalLight dirLight;
	class WaveTerrain *wave;
	class SimpleTerrain *terrain;
	class BasicCube *box;
	
	ID3D11Buffer *waveVB;
	ID3D11Buffer *waveIB;
	ID3D11Buffer *otherVB;
	ID3D11Buffer *otherIB;

	//move texture
	DirectX::XMMATRIX moveMatrix;
	DirectX::XMFLOAT2 moveOffset;
};