#pragma once
#include "WindowsApp.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

class DirectXApp : public WindowsApp
{
public:
	DirectXApp( HINSTANCE hinstance , int show );
	~DirectXApp();

	bool initDirectApp();

private:

	bool createDirectXDevice();
	void checkMSAAQuality();
	void createSwapChain();
	void createBackBufferView();
	void createDepthBufferView();

	ID3D11Device *device;
	ID3D11DeviceContext *immediateContext;
	IDXGISwapChain *swapChain;

	bool enable4xMSAA;
	UINT m4xMsaaQuality;

	ID3D11RenderTargetView *backBufferView;
	ID3D11DepthStencilView *depthBufferView;
};

