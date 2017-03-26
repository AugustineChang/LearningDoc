#include "Lights.fx"

cbuffer cbPerFrame
{
	DirectionalLight gDirectLight;
	PointLight gPointLight;
	SpotLight gSpotLight;
	float3 gCameraPosW;
};

cbuffer cbPerObject
{
	float4x4 gWVP;
	float4x4 gWorld;
	float4x4 gWorldNormal;
	float4x4 gTexTransform;
	Material gMaterial;
};

Texture2D diffuseTex;
Texture2D diffuseAlphaTex;
SamplerState linearSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	//MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

struct VertexIn
{
	float3 posL : POSITION;
	float3 normalL : NORMAL;
	float2 texcoord : TEXCOORD;
};

struct VertexOut
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float4 color : COLOR;
	float2 tex : TEXCOORD;
};



VertexOut VS( VertexIn vin )
{
	VertexOut vout;

	vout.posH = mul( float4( vin.posL , 1.0f ) , gWVP );
	vout.posW = mul( float4( vin.posL , 1.0f ) , gWorld ).xyz;
	vout.normalW = mul( vin.normalL , (float3x3)gWorldNormal );
	vout.color = float4( vin.normalL , 1.0f );
	vout.tex = mul( float4( vin.texcoord , 0.0f, 1.0f ) , gTexTransform ).xy;

	return vout;
}


float4 PS( VertexOut v2p , uniform bool isLit , uniform bool isUseTexture ) : SV_Target
{
	float4 texCol = float4( 1.0f , 1.0f , 1.0f , 1.0f );
	if ( isUseTexture )
	{
		// Sample texture.
		texCol = diffuseTex.Sample( linearSampler , v2p.tex );
		//float4 texAlphaCol = diffuseAlphaTex.Sample( linearSampler , v2p.tex );
		//texCol = texCol * texAlphaCol;
	}

	if ( isLit )
	{
		v2p.normalW = normalize( v2p.normalW );
		float3 viewW = normalize( gCameraPosW - v2p.posW );

		float4 diffuse = float4( 0.0f , 0.0f , 0.0f , 0.0f );
		float4 specular = float4( 0.0f , 0.0f , 0.0f , 0.0f );
		float4 ambient = float4( 0.0f , 0.0f , 0.0f , 0.0f );

		float4 A , D , S;
		ComputeDirectionalLight( gDirectLight , gMaterial , 1.0f , v2p.normalW , viewW , D , S , A );
		diffuse += D;
		specular += S;
		ambient += A;

		ComputePointLight( gPointLight , gMaterial , v2p.posW , 1.0f , v2p.normalW , viewW , D , S , A );
		diffuse += D;
		specular += S;
		ambient += A;

		ComputeSpotLight( gSpotLight , gMaterial , v2p.posW , 1.0f , v2p.normalW , viewW , D , S , A );
		diffuse += D;
		specular += S;
		ambient += A;

		float4 litColor = texCol * ( diffuse + ambient ) + specular;
		litColor.w = gMaterial.diffuse.w;
		return litColor;
	}
	
	return texCol * v2p.color;
}

technique11 LightTech_Lit_Tex
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetPixelShader( CompileShader( ps_5_0 , PS( true , true ) ) );
	}
}

technique11 LightTech_Lit_NoTex
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0 , VS() ) );
		SetPixelShader( CompileShader( ps_5_0 , PS( true , false ) ) );
	}
}

technique11 LightTech_Unlit_Tex
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0 , VS() ) );
		SetPixelShader( CompileShader( ps_5_0 , PS( false , true ) ) );
	}
}

technique11 LightTech_Unlit_NoTex
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0 , VS() ) );
		SetPixelShader( CompileShader( ps_5_0 , PS( false , false ) ) );
	}
}