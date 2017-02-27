#ifndef DIRECTX_APP_H
#define DIRECTX_APP_H

#include "../Utilities/WindowsApp.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11Texture2D;

class DirectXApp : public WindowsApp
{
public:
	DirectXApp( HINSTANCE hinstance , int show );
	~DirectXApp();

	virtual bool InitDirectApp();
	void QueryGraphicAdapters();

protected:

	virtual void OnResize() override;
	virtual void OnMouseDown( WPARAM btnState , int x , int y ) override;
	virtual void OnMouseMove( WPARAM btnState , int x , int y ) override;
	virtual void OnMouseUp( WPARAM btnState , int x , int y ) override;

	virtual void UpdateScene( float deltaTime ) override;
	virtual void DrawScene() override;

protected:

	ID3D11Device *device;
	ID3D11DeviceContext *immediateContext;
	IDXGISwapChain *swapChain;
	ID3D11Texture2D *depthBuffer;

	ID3D11RenderTargetView *backBufferView;
	ID3D11DepthStencilView *depthBufferView;
	
	bool enable4xMSAA;

private:

	bool createDirectXDevice();
	void checkMSAAQuality();
	void createSwapChain();
	void createBackBufferView();
	void createDepthBufferView();
	void initImmediateContext();

	UINT m4xMsaaQuality;
};

#endif // !DIRECTX_APP_H