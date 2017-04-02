#pragma once
#include "DirectXApp.h"
#include "Camera.h"
#include "Lights.h"
#include "ShaderEffect.h"

#include <vector>

struct CustomVertex;

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

	void createInputLayout();
	void createRenderState();
	
	std::vector<class BasicShape*> renderList;

	bool isMove;
	POINT lastMousePos;
	float moveSpeed;
	float radius;
	float zoomSpeed;

protected:

	void createObjects();
	void addToGlobalBuffer( std::vector<CustomVertex> &gVBuffer , std::vector<UINT> &gIBuffer , BasicShape &shape );
	
	template<typename T>
	void createVertexBuffer( const T *vertices , UINT vertexNum );
	void createIndexBuffer( const UINT *indices , UINT indexNum );
	void renderObject( const BasicShape &basicObj , UINT indexSize , UINT indexStart , UINT indexBase );
	
	Camera camera;
	ShaderEffect effect;
	DirectionalLight dirLight;
	PointLight pointLight;
	SpotLight spotLight;

	//DirectX::XMMATRIX rotMatrix;
	//float rotAngle;

	struct ID3D11InputLayout* inputLayout;
	struct ID3D11Buffer *vertexBuffer;
	struct ID3D11Buffer *indexBuffer;
	struct ID3D11RasterizerState *rasterState;
};