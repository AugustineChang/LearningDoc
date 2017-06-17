#pragma once
#include "DirectXApp.h"
#include "Camera.h"
#include "Lights.h"
#include <DirectXMath.h>
#include <vector>

struct BaseVertex;
struct ID3D11Buffer;
class BasicShape;

class TessellationHillScene : public DirectXApp
{
public:
	TessellationHillScene( HINSTANCE hinstance , int show );
	TessellationHillScene( const TessellationHillScene &copy ) = delete;
	TessellationHillScene& operator=( const TessellationHillScene &copy ) = delete;
	~TessellationHillScene();

	virtual bool InitDirectApp() override;
	virtual void UpdateScene( float deltaTime ) override;
	virtual void DrawScene() override;

	virtual void OnResize() override;
	virtual void OnMouseDown( WPARAM btnState , int x , int y ) override;
	virtual void OnMouseMove( WPARAM btnState , int x , int y ) override;
	virtual void OnMouseUp( WPARAM btnState , int x , int y ) override;
	virtual void OnMouseWheel( int zDelta ) override;

	virtual void OnKeyDown( WPARAM keyCode ) override;
	virtual void OnKeyUp( WPARAM keyCode ) override;

private:

	bool isMove;
	POINT lastMousePos;
	float rotSpeed;
	float moveSpeed;

	bool isWDown;
	bool isADown;
	bool isSDown;
	bool isDDown;

protected:

	void createObjects();
	template<typename T>
	void createVertexBuffer( const T *vertices , UINT vertexNum , ID3D11Buffer *&vertexBuffer );

	Camera camera;
	DirectionalLight dirLight;
	class BasicQuad *terrain;
	
	ID3D11Buffer *waveVB;
};