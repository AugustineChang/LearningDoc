#include "Lights.fx"

cbuffer cbPerFrame
{
	DirectionalLight gDirectLight;
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


float4 PS( VertexOut v2p , uniform bool isLit , uniform bool isUseTexture , 
	uniform bool isUseFog ) : SV_Target
{
	float4 texCol = float4( 1.0f , 1.0f , 1.0f , 1.0f );
	if ( isUseTexture )
	{
		// Sample texture.
		texCol = diffuseTex.Sample( linearSampler , v2p.tex );
		clip( texCol.a - 0.1f );
	}

	float4 litColor = float4( 0.0f , 0.0f , 0.0f , 0.0f );
	if ( isLit )
	{
		v2p.normalW = normalize( v2p.normalW );
		float3 viewW = gCameraPosW - v2p.posW;
		float dist2View = length( viewW );
		viewW /= dist2View;

		float4 diffuse = float4( 0.0f , 0.0f , 0.0f , 0.0f );
		float4 specular = float4( 0.0f , 0.0f , 0.0f , 0.0f );
		float4 ambient = float4( 0.0f , 0.0f , 0.0f , 0.0f );

		ComputeDirectionalLight( gDirectLight , gMaterial , 1.0f , v2p.normalW , viewW , diffuse , specular , ambient );
		litColor.w = gMaterial.diffuse.w;

		litColor = texCol * ( diffuse + ambient ) + specular;
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
		SetVertexShader( CompileShader( vs_5_0 , VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0 , PS( true , true , false ) ) );
	}
}

technique11 LightTech_Lit_Tex_Fog
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0 , VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0 , PS( true , true , true ) ) );
	}
}

technique11 LightTech_Lit
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0 , VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0 , PS( true , false , false ) ) );
	}
}

technique11 LightTech_Unlit_Tex
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0 , VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0 , PS( false , true , false ) ) );
	}
}

technique11 LightTech_Unlit
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0 , VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0 , PS( false , false , false ) ) );
	}
}