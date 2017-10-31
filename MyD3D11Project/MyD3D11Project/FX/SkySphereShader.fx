cbuffer cbPerObject
{
	float4x4 gWVP;
};

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
	float3 posL : POSITION;
};


VertexOut VS( VertexIn vin )
{
	VertexOut vout;

	vout.posH = mul( float4( vin.posL , 1.0f ) , gWVP );
	vout.posL = vin.posL;

	return vout;
}


float4 PS( VertexOut v2p , uniform bool isUseTexture ) : SV_Target
{
	float4 texCol = float4( 1.0f , 1.0f , 1.0f , 1.0f );
	if ( isUseTexture )
	{
		// Sample texture.
		texCol = cubeMap.Sample( linearSampler , v2p.posL );
	}
	
	return texCol;
}

technique11 CubemapTech_Tex
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0 , PS( true ) ) );
	}
}

technique11 CubemapTech_NoTex
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0 , VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0 , PS( false ) ) );
	}
}