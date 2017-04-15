cbuffer cbPerObject
{
	float4x4 gWVP;
};


struct VertexIn
{
	float3 posL : POSITION;
	float3 normal : NORMAL;
};

struct VertexOut
{
	float4 posH : SV_POSITION;
	float4 color : COLOR;
};



VertexOut VS( VertexIn vin )
{
	VertexOut vout;

	vout.posH = mul( float4( vin.posL , 1.0f ) , gWVP );
	vout.color = float4( vin.normal , 1.0f );

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
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0 , PS() ) );
	}
}