#ifndef UTILITIES_H
#define UTILITIES_H

#include <dxerr.h>
#include <DirectXMath.h>

#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)                                              \
	{                                                          \
		HRESULT hr = (x);                                      \
		if(FAILED(hr))                                         \
		{                                                      \
			DXTrace(__FILEW__, (DWORD)__LINE__, hr, L#x, true); \
		}                                                      \
	}
#endif

#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 


#define ReleaseCOM(x) { if(x){ x->Release(); x = 0; } }

namespace Colors
{
	XMGLOBALCONST DirectX::XMVECTORF32 White = { 1.0f,1.0f,1.0f,1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Black = { 0.0f,0.0f,0.0f,1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Red = { 1.0f,0.0f,0.0f,1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Green = { 0.0f,1.0f,0.0f,1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Blue = { 0.0f,0.0f,1.0f,1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
}

#endif // !UTILITIES_H

struct BaseVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexCoord;
	DirectX::XMFLOAT3 TangentU;
};

struct TessVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 TexCoord;
};

struct InstanceData
{
	DirectX::XMFLOAT4X4 World;
	DirectX::XMFLOAT4 Color;
};

struct CustomMaterial
{
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;// w = SpecPower
};