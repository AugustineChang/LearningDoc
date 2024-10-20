#pragma once
#include "DirectXApp.h"
#include "Camera.h"
#include "Lights.h"
#include "ShaderEffect.h"

#include <vector>

class BasicShape;
class DebugQuad;
struct BaseVertex;
struct ID3D11Buffer;
struct ID3D11DepthStencilState;

class SimpleScene : public DirectXApp
{
public:
	SimpleScene( HINSTANCE hinstance , int show );
	SimpleScene( const SimpleScene &copy ) = delete;
	SimpleScene& operator=( const SimpleScene &copy ) = delete;
	~SimpleScene();

	virtual bool InitDirectApp() override;
	virtual void UpdateScene( float deltaTime ) override;
	virtual void DrawScene() override;

	virtual void OnResize() override;
	virtual void OnMouseDown( WPARAM btnState , int x , int y ) override;
	virtual void OnMouseMove( WPARAM btnState , int x , int y ) override;
	virtual void OnMouseUp( WPARAM btnState , int x , int y ) override;
	virtual void OnMouseWheel( int zDelta ) override;

private:
		
	std::vector<BasicShape*> renderList;
	DebugQuad *debugDepth;

	bool isMove;
	POINT lastMousePos;
	float moveSpeed;
	float radius;
	float zoomSpeed;

protected:

	BasicShape *curPickObj;
	float pickDistance;
	void pickupObj( float x , float y );
	void dropObj();

	void createObjects();
	void addToGlobalBuffer( std::vector<BaseVertex> &gVBuffer , std::vector<UINT> &gIBuffer , BasicShape &shape );
	
	template<typename T>
	void createVertexBuffer( const T *vertices , UINT vertexNum );
	void createIndexBuffer( const UINT *indices , UINT indexNum );
	void createDepthStencilState();

	Camera camera;
	DirectionalLight dirLight[2];
	
	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer;
	ID3D11DepthStencilState *depthState;
};