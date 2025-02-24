#include "Lights.fx"

cbuffer cbPerFrame
{
	float3 gCameraPosW;
};

cbuffer cbPerObject
{
	float4x4 gWVP;
	float4x4 gWorld;
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
	float2 texcoord : TEXCOORD;
};

struct VertexOut
{
	float3 posL : POSITION;
	float2 texcoord : TEXCOORD;
};

VertexOut VS( VertexIn vin )
{
	VertexOut vout;

	vout.posL = vin.posL;
	vout.texcoord = vin.texcoord;

	return vout;
}

struct PatchTess
{
	float edgeTess[4] : SV_TessFactor;
	float insideTess[2] : SV_InsideTessFactor;
};

//PatchTess ConstantHS( InputPatch<VertexOut,4> patch, uint patchID : SV_PrimitiveID )
PatchTess ConstantHS( InputPatch<VertexOut , 16> patch , uint patchID : SV_PrimitiveID )
{
	PatchTess ps;

	/*float3 center = ( patch[0].posL + patch[1].posL + patch[2].posL + patch[3].posL ) * 0.25f;
	float3 centerW = mul( float4( center , 1.0f ) , gWorld ).xyz;

	float dist = distance( centerW , gCameraPosW );
	
	float max = 100.0f;
	float min = 20.0f;
	float tess = 64.0f * saturate( ( max - dist ) / ( max - min ) );*/

	ps.edgeTess[0] = 25;
	ps.edgeTess[1] = 25;
	ps.edgeTess[2] = 25;
	ps.edgeTess[3] = 25;

	ps.insideTess[0] = 25;
	ps.insideTess[1] = 25;

	return ps;
}

struct HullOut
{
	float3 posL : POSITION;
	float2 texcoord : TEXCOORD;
};

[domain( "quad" )]
[partitioning( "integer" )]
[outputtopology( "triangle_cw" )]
//[outputcontrolpoints( 4 )]
[outputcontrolpoints( 16 )]
[patchconstantfunc( "ConstantHS" )]
[maxtessfactor( 64.0f )]
//HullOut HS( InputPatch<VertexOut,4> patch , 
HullOut HS( InputPatch<VertexOut,16> patch , 
	uint controlPointIndex : SV_OutputControlPointID,
	uint patchID : SV_PrimitiveID)
{
	HullOut hout;

	hout.posL = patch[controlPointIndex].posL;
	hout.texcoord = patch[controlPointIndex].texcoord;

	return hout;
}

///////////////////Cubic Bezier Surface //////////////////////

float4 BernsteinBasic( float t )
{
	float invT = 1.0f - t;
	return float4
		(
			invT * invT * invT,
			3.0f * t * invT * invT,
			3.0f * t * t * invT,
			t * t * t
		);
}

float4 dBernsteinBasic( float t )
{
	float invT = 1.0f - t;
	return float4
		(
			-3.0f * invT * invT ,
			3.0f * invT * invT - 6.0f * t * invT ,
			6.0f * t * invT - 3 * t * t,
			3.0f * t * t
			);
}

float3 CubicBezierSum( const OutputPatch<HullOut , 16> bezPath , float4 basisU , float4 basisV )
{
	float3 sum = float3( 0.0f , 0.0f , 0.0f );
	sum = basisV.x * ( 
		basisU.x * bezPath[0].posL +
		basisU.y * bezPath[1].posL +
		basisU.z * bezPath[2].posL +
		basisU.w * bezPath[3].posL );

	sum += basisV.y * ( 
		basisU.x * bezPath[4].posL +
		basisU.y * bezPath[5].posL +
		basisU.z * bezPath[6].posL +
		basisU.w * bezPath[7].posL );

	sum += basisV.z * (
		basisU.x * bezPath[8].posL +
		basisU.y * bezPath[9].posL +
		basisU.z * bezPath[10].posL +
		basisU.w * bezPath[11].posL );

	sum += basisV.w * (
		basisU.x * bezPath[12].posL +
		basisU.y * bezPath[13].posL +
		basisU.z * bezPath[14].posL +
		basisU.w * bezPath[15].posL );

	return sum;
}

//////////////////////////////////////////////////////////////

struct DomainOut
{
	float4 posH : SV_POSITION;
	float2 texUV : TEXCOORD;
};

[domain( "quad" )]
DomainOut DS( PatchTess patchTess , 
	float2 domainUV : SV_DomainLocation ,
	//const OutputPatch<HullOut,4> quad )
	const OutputPatch<HullOut,16> bezPatch )
{
	DomainOut dout;

	/*float3 v1 = lerp( quad[0].posL , quad[1].posL , domainUV.x );
	float3 v2 = lerp( quad[2].posL , quad[3].posL , domainUV.x );
	float3 curPoint = lerp( v1 , v2 , domainUV.y );
	curPoint.y = 0.3f*( curPoint.z * sin( curPoint.x ) + curPoint.x * cos( curPoint.z ) );*/

	float4 basisU = BernsteinBasic( domainUV.x );
	float4 basisV = BernsteinBasic( domainUV.y );
	float3 curPoint = CubicBezierSum( bezPatch , basisU , basisV );

	//float2 uv1 = lerp( quad[0].texcoord , quad[1].texcoord , 1 - domainUV.x );
	//float2 uv2 = lerp( quad[2].texcoord , quad[3].texcoord , 1 - domainUV.x );
	float2 uv1 = lerp( bezPatch[0].texcoord , bezPatch[3].texcoord , 1 - domainUV.x );
	float2 uv2 = lerp( bezPatch[12].texcoord , bezPatch[15].texcoord , 1 - domainUV.x );
	float2 curUV = lerp( uv1 , uv2 , domainUV.y );

	dout.posH = mul( float4( curPoint , 1.0f ) , gWVP );
	dout.texUV = mul( float4( curUV , 0.0f , 1.0f ) , gTexTransform ).xy;

	return dout;
}

float4 PS( DomainOut d2p , uniform bool isUseTexture ) : SV_Target
{
	float4 texCol = float4( 1.0f , 1.0f , 1.0f , 1.0f );
	if ( isUseTexture )
	{
		texCol = diffuseTex.Sample( linearSampler , d2p.texUV );
	}
	texCol.w = gMaterial.diffuse.w;

	return texCol;
}

technique11 TessTech
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetHullShader( CompileShader( hs_5_0 , HS() ) );
		SetDomainShader( CompileShader( ds_5_0 , DS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0 , PS( true ) ) );
	}
}