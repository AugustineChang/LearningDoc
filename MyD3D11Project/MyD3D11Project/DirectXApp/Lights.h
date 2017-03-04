#pragma once
#include <windows.h>
#include <DirectXMath.h>
#include "../Utilities/SimpleMath.h"

struct DirectionalLight
{
	DirectionalLight()
	{
		diffuseColor = DirectX::XMFLOAT4( 0.3686f , 0.8784f , 0.8901f , 1.0f );
		ambientColor = SimpleMath::Mul( diffuseColor , 0.3f );
		specularColor = SimpleMath::Mul( diffuseColor , 0.5f );

		DirectX::XMVECTOR dir = DirectX::XMVectorSet( 1.0f , -1.0f , 0.5f , 0.0f );
		DirectX::XMStoreFloat3( &direction , DirectX::XMVector3Normalize( dir ) );
	}

	DirectX::XMFLOAT3 direction;
	float pad;

	DirectX::XMFLOAT4 ambientColor;
	DirectX::XMFLOAT4 diffuseColor;
	DirectX::XMFLOAT4 specularColor;
};

struct PointLight
{
	PointLight() 
	{ 
		diffuseColor = DirectX::XMFLOAT4( 0.3686f , 0.8784f , 0.8901f , 1.0f );
		ambientColor = SimpleMath::Mul( diffuseColor , 0.3f );
		specularColor = SimpleMath::Mul( diffuseColor , 0.5f );
		position = DirectX::XMFLOAT3( 0.0f , 3.0f , 0.0f );
		range = 25.0f;
		attenu = DirectX::XMFLOAT3( 0.0f , 1.0f , 0.0f );
	}

	DirectX::XMFLOAT3 position;
	float range;

	DirectX::XMFLOAT4 ambientColor;
	DirectX::XMFLOAT4 diffuseColor;
	DirectX::XMFLOAT4 specularColor;

	DirectX::XMFLOAT3 attenu;
	float pad;
};

struct SpotLight
{
	SpotLight()
	{
		diffuseColor = DirectX::XMFLOAT4( 0.3686f , 0.8784f , 0.8901f , 1.0f );
		ambientColor = SimpleMath::Mul( diffuseColor , 0.3f );
		specularColor = SimpleMath::Mul( diffuseColor , 0.5f );
		position = DirectX::XMFLOAT3( 2.0f , 1.0f , 2.0f );

		DirectX::XMVECTOR dir = DirectX::XMVectorSet( -2.0f , -1.0f , -2.0f , 0.0f );
		DirectX::XMStoreFloat3( &direction , DirectX::XMVector3Normalize( dir ) );

		range = 225.0f;
		attenu = DirectX::XMFLOAT3( 2.0f , 0.0f , 0.0f );
		spot = 10.0f;
	}

	DirectX::XMFLOAT3 position;
	float range;

	DirectX::XMFLOAT3 direction;
	float spot;

	DirectX::XMFLOAT4 ambientColor;
	DirectX::XMFLOAT4 diffuseColor;
	DirectX::XMFLOAT4 specularColor;

	DirectX::XMFLOAT3 attenu;
	float pad;
};