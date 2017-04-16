#include "DebugQuad.h"
#include "../Utilities/CommonHeader.h"
#include "../DirectXApp/Camera.h"
#include "DDSTextureLoader.h"
using namespace DirectX;


DebugQuad::DebugQuad() : BasicShape( "DebugShader" )
{
	isUseGlobalBuffer = false;
	isEnableFog = false;
	techName = "DebugTech";
}


DebugQuad::~DebugQuad()
{
}

void DebugQuad::RenderObject( ID3D11DeviceContext *immediateContext )
{
	immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
	immediateContext->IASetInputLayout( nullptr );
	immediateContext->RSSetState( rasterState );

	ID3DX11EffectTechnique *technique = effect.getEffectTech( techName.c_str() );
	D3DX11_TECHNIQUE_DESC techDesc;
	technique->GetDesc( &techDesc );
	for ( UINT i = 0; i < techDesc.Passes; ++i )
	{
		technique->GetPassByIndex( i )->Apply( 0 , immediateContext );

		immediateContext->Draw( 4 , 0 );
	}
}

void DebugQuad::UpdateDebugTexture( ID3D11ShaderResourceView *texView )
{
	efDepthBuf->SetResource( texView );
}

void DebugQuad::createEffect( ID3D11Device *device )
{
	effect.createEffectAtBuildtime( device );

	efDepthBuf = effect.getEffect()->GetVariableByName( "depthBuffer" )->AsShaderResource();
}
