#include "Lights.fx"

cbuffer cbPerFrame
{
	DirectionalLight gDirectLight[3];
	PointLight gPointLight[3];
	SpotLight gSpotLight[3];
	float gDirLightNum;
	float gPointLightNum;
	float gSpotLightNum;

	float3 gCameraPosW;

	float gFogStart;
	float gFogDistance;
	float4 gFogColor;
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
TextureCube cubeMap;
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


void UpdateLights( float3 pos , float4 color , float3 normal , float3 view , out float4 diffuse , out float4 specular , out float4 ambient )
{
	diffuse = float4( 0.0f , 0.0f , 0.0f , 0.0f );
	specular = float4( 0.0f , 0.0f , 0.0f , 0.0f );
	ambient = float4( 0.0f , 0.0f , 0.0f , 0.0f );

	if ( gDirLightNum > 0 )
	{
		[unroll]
		for ( int i = 0; i < gDirLightNum; ++i )
		{
			float4 A , D , S;
			ComputeDirectionalLight( gDirectLight[i] , gMaterial , color , normal , view , D , S , A );
			diffuse += D;
			specular += S;
			ambient += A;
		}
	}
	
	if ( gPointLightNum )
	{
		[unroll]
		for ( int i = 0; i < gPointLightNum; ++i )
		{
			float4 A , D , S;
			ComputePointLight( gPointLight[i] , gMaterial , pos , color , normal , view , D , S , A );
			diffuse += D;
			specular += S;
			ambient += A;
		}
	}
	
	if ( gSpotLightNum )
	{
		[unroll]
		for ( int i = 0; i < gSpotLightNum; ++i )
		{
			float4 A , D , S;
			ComputeSpotLight( gSpotLight[i] , gMaterial , pos , color , normal , view , D , S , A );
			diffuse += D;
			specular += S;
			ambient += A;
		}
	}
}

float4 PS( VertexOut v2p , uniform bool isLit , uniform bool isUseTexture ,
	uniform bool isUseFog ) : SV_Target
{
	float4 texCol = float4( 1.0f , 1.0f , 1.0f , 1.0f );
	if ( isUseTexture )
	{
		// Sample texture.
		texCol = diffuseTex.Sample( linearSampler , v2p.tex );
	}

	float4 litColor = float4( 0.0f , 0.0f , 0.0f , 0.0f );
	if ( isLit )
	{
		v2p.normalW = normalize( v2p.normalW );
		float3 viewW = gCameraPosW - v2p.posW;
		float dist2View = length( viewW );
		viewW /= dist2View;

		float4 diffuse, specular, ambient;
		UpdateLights( v2p.posW , 1.0f , v2p.normalW , viewW , diffuse , specular , ambient );

		litColor = texCol * ( diffuse + ambient ) + specular;
		litColor.w = gMaterial.diffuse.w;

		float reflectFactor = 1.0f - dot( viewW , v2p.normalW );
		float3 reflectW = reflect( -viewW , v2p.normalW );
		float4 reflectCol = cubeMap.Sample( linearSampler , reflectW );

		litColor = ( 1.0f - reflectFactor ) * litColor + reflectFactor * reflectCol;

		if ( isUseFog )
		{
			float fogFactor = ( dist2View - gFogStart ) / gFogDistance;
			fogFactor = saturate( fogFactor );
			litColor = lerp( litColor , gFogColor , fogFactor );
		}
	}
	else
	{
		litColor = texCol * v2p.color;
	}

	return litColor;
}

technique11 LightTech_Lit_Tex
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0 , PS( true , true , false ) ) );
	}
}

technique11 LightTech_Lit_Tex_Fog
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0 , VS() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0 , PS( true , true , true ) ) );
	}
}

technique11 LightTech_Lit_NoTex
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0 , VS() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0 , PS( true , false , false ) ) );
	}
}

technique11 LightTech_Unlit_Tex
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0 , VS() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0 , PS( false , true , false ) ) );
	}
}

technique11 LightTech_Unlit_NoTex
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0 , VS() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0 , PS( false , false , false ) ) );
	}
}