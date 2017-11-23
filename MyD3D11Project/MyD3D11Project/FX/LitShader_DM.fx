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
	float4x4 gVP;
	float4x4 gWorldNormal;
	float4x4 gTexTransform;
	Material gMaterial;
};

Texture2D diffuseTex;
Texture2D normalTex;
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
	float3 tangentL : TANGENT;
};

struct VertexOut
{
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float2 tex : TEXCOORD;
	float tessFactor : TESS;
};

float minTessDis = 5.0f;
float maxTessDis = 1.0f;
float minTessFactor = 1.0f;
float maxTessFactor = 20.0f;

VertexOut VS( VertexIn vin )
{
	VertexOut vout;

	vout.posW = mul( float4( vin.posL , 1.0f ) , gWorld ).xyz;
	vout.normalW = mul( vin.normalL , ( float3x3 )gWorldNormal );
	vout.tangentW = mul( vin.tangentL , (float3x3)gWorld );
	vout.tex = mul( float4( vin.texcoord , 0.0f, 1.0f ) , gTexTransform ).xy;
	
	float dist = distance( vout.posW , gCameraPosW );

	float tess = saturate( ( minTessDis - dist ) / ( minTessDis - maxTessDis ) );
	vout.tessFactor = minTessFactor + tess * ( maxTessFactor - minTessFactor );

	return vout;
}


struct PatchTess
{
	float EdgeTess[3] : SV_TessFactor;
	float InsideTess : SV_InsideTessFactor;
};

PatchTess ConstantHS( InputPatch<VertexOut , 3> patch , uint patchID : SV_PrimitiveID )
{
	PatchTess pt;

	pt.EdgeTess[0] = 0.5f * ( patch[1].tessFactor + patch[2].tessFactor );
	pt.EdgeTess[1] = 0.5f * ( patch[2].tessFactor + patch[0].tessFactor );
	pt.EdgeTess[2] = 0.5f * ( patch[0].tessFactor + patch[1].tessFactor );
	pt.InsideTess = ( pt.EdgeTess[0] + pt.EdgeTess[1] + pt.EdgeTess[2] ) / 3.0f;

	return pt;
}

struct HullOut
{
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float2 texcoord : TEXCOORD;
	float3 tangentW : TANGENT;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantHS")]
HullOut ControlPointHS( InputPatch<VertexOut , 3> patch , uint i : SV_OutputControlPointID ,
	uint patchID : SV_PrimitiveID )
{
	HullOut hout;

	hout.posW = patch[i].posW;
	hout.normalW = patch[i].normalW;
	hout.texcoord = patch[i].tex;
	hout.tangentW = patch[i].tangentW;

	return hout;
}

struct DomainOut 
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float2 tex : TEXCOORD;
};

[domain("tri")]
DomainOut DS( PatchTess patchTess , float3 bary : SV_DomainLocation , const OutputPatch<HullOut , 3> triangles )
{
	DomainOut dout;

	dout.posW = triangles[0].posW * bary.x + triangles[1].posW * bary.y + triangles[2].posW * bary.z;
	dout.normalW = triangles[0].normalW * bary.x + triangles[1].normalW * bary.y + triangles[2].normalW * bary.z;
	dout.tangentW = triangles[0].tangentW * bary.x + triangles[1].tangentW * bary.y + triangles[2].tangentW * bary.z;
	dout.tex = triangles[0].texcoord * bary.x + triangles[1].texcoord * bary.y + triangles[2].texcoord * bary.z;

	dout.normalW = normalize( dout.normalW );

	float mipInterval = 20.0f;
	float mipLevel = clamp( ( distance( dout.posW , gCameraPosW ) - mipInterval ) / mipInterval , 0.0f , 6.0f );

	float height = normalTex.SampleLevel( linearSampler , dout.tex , mipLevel ).a;
	dout.posW += 0.1f * ( height - 0.5f ) * dout.normalW;
	
	dout.posH = mul( float4( dout.posW , 1.0f ) , gVP );

	return dout;
}

void UpdateLights( float3 pos , float3 normal , float3 view , out float4 diffuse , out float4 specular , out float4 ambient )
{
	diffuse = float4( 0.0f , 0.0f , 0.0f , 0.0f );
	specular = float4( 0.0f , 0.0f , 0.0f , 0.0f );
	ambient = float4( 0.0f , 0.0f , 0.0f , 0.0f );

	float4 color = float4( 1.0f , 1.0f , 1.0f , 1.0f );

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


float4 PS( DomainOut d2p , uniform bool isLit , uniform bool isUseTexture ,
	uniform bool isUseFog , uniform bool isUseNormalMap ) : SV_Target
{
	float4 texCol = float4( 1.0f , 1.0f , 1.0f , 1.0f );
	if ( isUseTexture )
	{
		// Sample texture.
		texCol = diffuseTex.Sample( linearSampler , d2p.tex );
	}

	float4 litColor = float4( 0.0f , 0.0f , 0.0f , 0.0f );
	if ( isLit )
	{
		d2p.normalW = normalize( d2p.normalW );
		float3 viewW = gCameraPosW - d2p.posW;
		float dist2View = length( viewW );
		viewW /= dist2View;

		float4 diffuse, specular, ambient;
		
		if ( isUseNormalMap )
		{
			float3 normalSample = normalTex.Sample( linearSampler , d2p.tex ).rgb;
			float3 bumpedNormal = TangentToWorld( normalSample , d2p.normalW , d2p.tangentW );
			UpdateLights( d2p.posW , bumpedNormal , viewW , diffuse , specular , ambient );
		}
		else
		{
			UpdateLights( d2p.posW , d2p.normalW , viewW , diffuse , specular , ambient );
		}

		litColor = texCol * ( diffuse + ambient ) + specular;
		litColor.w = gMaterial.diffuse.w;

		if ( isUseFog )
		{
			float fogFactor = ( dist2View - gFogStart ) / gFogDistance;
			fogFactor = saturate( fogFactor );
			litColor = lerp( litColor , gFogColor , fogFactor );
		}
	}
	else
	{
		litColor = texCol;
	}

	return litColor;
}

technique11 LightTech_DisplacementMap
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0 , VS() ) );
		SetHullShader( CompileShader( hs_5_0 , ControlPointHS() ) );
		SetDomainShader( CompileShader( ds_5_0 , DS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0 , PS( true , true , true , true ) ) );
	}
}