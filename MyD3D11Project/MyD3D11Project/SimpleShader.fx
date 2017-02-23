cbPerObject
{
	gWVP;
};


struct VertexIn
{
	float3 posL : Position;
	float4 color : Color;
};

struct VertexOut
{
	float4 posH : SV_Position;
	float4 color : Color;
};



VertexOut VS( VertexIn in )
{
	VertexOut vout;

	vout.posH = mul( float4( in.posL , 1.0f ) , gWVP );
	vout.color = in.color;

	return vout;
}


float4 PS( VertexOut v2p ) : SV_Target
{
	return v2p.color;
}

technique11 SimpleTech
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetPixelShader( CompileShader( ps_5_0 , PS() ) );
	}
}