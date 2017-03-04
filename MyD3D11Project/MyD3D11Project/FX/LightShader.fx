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
	Material gMaterial;
};


struct VertexIn
{
	float3 posL : POSITION;
	float3 normalL : NORMAL;
};

struct VertexOut
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float4 color : COLOR;
};



VertexOut VS( VertexIn vin )
{
	VertexOut vout;

	vout.posH = mul( float4( vin.posL , 1.0f ) , gWVP );
	vout.posW = mul( float4( vin.posL , 1.0f ) , gWorld ).xyz;
	vout.normalW = mul( vin.normalL , (float3x3)gWorldNormal );
	vout.color = float4( vin.normalL , 1.0f );

	return vout;
}


float4 PS( VertexOut v2p ) : SV_Target
{
	v2p.normalW = normalize( v2p.normalW );
	float3 viewW = normalize( gCameraPosW - v2p.posW );

	float4 diffuse = float4( 0.0f , 0.0f , 0.0f , 0.0f );
	float4 specular = float4( 0.0f , 0.0f , 0.0f , 0.0f );
	float4 ambient = float4( 0.0f , 0.0f , 0.0f , 0.0f );
	
	float4 A , D , S;

	ComputeDirectionalLight( gDirectLight , gMaterial , v2p.color , v2p.normalW , viewW , D , S , A );
	diffuse += D;
	specular += S;
	ambient += A;

	ComputePointLight( gPointLight , gMaterial , v2p.posW , v2p.color , v2p.normalW , viewW , D , S , A );
	diffuse += D;
	specular += S;
	ambient += A;

	ComputeSpotLight( gSpotLight , gMaterial , v2p.posW , v2p.color , v2p.normalW , viewW , D , S , A );
	diffuse += D;
	specular += S;
	ambient += A;

	return float4( diffuse.xyz + specular.xyz + ambient.xyz , gMaterial.diffuse.w );
}

technique11 LightTech
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetPixelShader( CompileShader( ps_5_0 , PS() ) );
	}
}