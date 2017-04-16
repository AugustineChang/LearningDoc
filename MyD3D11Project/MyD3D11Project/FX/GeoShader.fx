cbuffer cbPerFrame
{
	float3 gCameraPosW;
	float3 gCameraUpW;
};

cbuffer cbPerObject
{
	float4x4 gVP;
	float4x4 gWorld;
	float4x4 gTexTransform;
};

cbuffer cbFixed
{
	float2 fixedUV[4] =
	{
		float2( 0.0f, 1.0f ),
		float2( 0.0f, 0.0f ),
		float2( 1.0f, 1.0f ),
		float2( 1.0f, 0.0f )
	};
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
	float3 centerL : POSITION;
	float2 size : SIZE;
};

struct VertexOut
{
	float3 centerW : POSITION;
	float2 size : SIZE;
};

struct GeoOut
{
	float4 posH : SV_POSITION;
	float2 tex : TEXCOORD;
};


VertexOut VS( VertexIn vin )
{
	VertexOut vout;

	vout.centerW = mul( float4( vin.centerL , 1.0f ) , gWorld ).xyz;
	vout.size = vin.size;

	return vout;
}


[maxvertexcount( 4 )]
void GS( point VertexOut v2g[1] , inout TriangleStream<GeoOut> triStream )
{
	float3 up = gCameraUpW;
	float3 look = normalize( gCameraPosW - v2g[0].centerW );
	float3 right = cross( up , look );

	float halfWidth = 0.5f * v2g[0].size.x;
	float halfHeight = 0.5f * v2g[0].size.y;

	//1---0
	//|   |
	//3---2
	float4 pointsW[4];
	pointsW[0] = float4( v2g[0].centerW + halfWidth*right + halfHeight*up , 1.0f );
	pointsW[1] = float4( v2g[0].centerW - halfWidth*right + halfHeight*up , 1.0f );
	pointsW[2] = float4( v2g[0].centerW + halfWidth*right - halfHeight*up , 1.0f );
	pointsW[3] = float4( v2g[0].centerW - halfWidth*right - halfHeight*up , 1.0f );

	GeoOut gOut;
	[unroll]
	for ( int i = 0; i < 4; ++i )
	{
		gOut.posH = mul( pointsW[i] , gVP );
		gOut.tex = mul( float4( fixedUV[i] , 0.0f , 1.0f ) , gTexTransform ).xy;

		triStream.Append( gOut );
	}
}

float4 PS( GeoOut g2p ) : SV_Target
{
	float4 texCol = diffuseTex.Sample( linearSampler , g2p.tex );
	return texCol;
}



technique11 GeoTech_Tex
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0 , GS() ) );
		SetPixelShader( CompileShader( ps_5_0 , PS() ) );
	}
}