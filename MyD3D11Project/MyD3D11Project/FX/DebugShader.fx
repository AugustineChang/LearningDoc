
DepthStencilState DSS
{
	DepthEnable = true;
	DepthWriteMask = Zero;
	StencilEnable = true;
	StencilReadMask = 0xff;
	StencilWriteMask = 0xff;
	FrontFaceStencilFunc = Always;
	FrontFaceStencilPass = Keep;
	FrontFaceStencilFail = Keep;
	BackFaceStencilFunc = Always;
	BackFaceStencilPass = Keep;
	BackFaceStencilFail = Keep;
};

Texture2D depthBuffer;
SamplerState linearSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	//MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

struct VertexOut
{
	float4 posH : SV_POSITION;
	float2 tex : TEXCOORD;
};

VertexOut VS( uint vI : SV_VERTEXID )
{
	VertexOut vout;

	float2 texcoord = float2( vI & 1 , vI >> 1 );
	vout.tex = texcoord;
	vout.posH = float4( ( texcoord.x - 0.5f ) * 2 , -( texcoord.y - 0.5f ) * 2 , 0 , 1 );

	return vout;
}

float4 PS( VertexOut v2p ) : SV_Target
{
	float4 depth = depthBuffer.Sample( linearSampler , v2p.tex );
	return float4( depth.rg , depth.b , 1 );
}



technique11 DebugTech
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0 , PS() ) );
	}
}