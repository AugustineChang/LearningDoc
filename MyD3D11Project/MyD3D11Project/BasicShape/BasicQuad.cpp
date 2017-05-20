#include "BasicQuad.h"
#include "../Utilities/CommonHeader.h"
#include "../DirectXApp/Camera.h"
#include "DDSTextureLoader.h"
using namespace DirectX;

BasicQuad::BasicQuad() : BasicShape( "TessellationShader" ) , width( 10.0f ) , height( 10.0f )
{
	techName = "TessTech";
}

BasicQuad::~BasicQuad()
{
}

void BasicQuad::UpdateObject( float DeltaTime )
{
}

void BasicQuad::createEffect( ID3D11Device *device )
{
	effect.createEffectAtBuildtime( device );

	efWVP = effect.getEffect()->GetVariableByName( "gWVP" )->AsMatrix();
	efWorld = effect.getEffect()->GetVariableByName( "gWorld" )->AsMatrix();
	efTexTrans = effect.getEffect()->GetVariableByName( "gTexTransform" )->AsMatrix();

	efMaterial = effect.getEffect()->GetVariableByName( "gMaterial" );
	efTexture = effect.getEffect()->GetVariableByName( "diffuseTex" )->AsShaderResource();
	
	efCameraPos = effect.getEffect()->GetVariableByName( "gCameraPosW" )->AsVector();
}

void BasicQuad::createObjectMesh()
{
	XMFLOAT3 zero = XMFLOAT3( 0.0f , 0.0f , 0.0f );

	tessVertices =
	{
		{ XMFLOAT3( width,0.0f,height ), XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( -width,0.0f,height ), XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( width,0.0f,-height ), XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( -width,0.0f,-height ), XMFLOAT2( 0.0f, 1.0f ) }
	};
}

void BasicQuad::createInputLayout( ID3D11Device *device )
{
	D3D11_INPUT_ELEMENT_DESC descList[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};

	D3DX11_PASS_DESC passDesc;
	effect.getEffectTech( techName.c_str() )->GetPassByIndex( 0 )->GetDesc( &passDesc );

	HR( device->CreateInputLayout( descList , 2 , passDesc.pIAInputSignature , passDesc.IAInputSignatureSize , &inputLayout ) );
}

void BasicQuad::createRenderState( ID3D11Device *device )
{
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory( &rsDesc , sizeof( D3D11_RASTERIZER_DESC ) );
	rsDesc.FillMode = D3D11_FILL_WIREFRAME;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;

	device->CreateRasterizerState( &rsDesc , &rasterState );
}

void BasicQuad::createObjectTexture( ID3D11Device *device )
{
	CreateDDSTextureFromFile( device , L"Textures/grass.dds" , &texture , &textureView );
}

void BasicQuad::UpdateObjectEffect( const Camera *camera )
{
	buildWorldMatrix();

	XMMATRIX &tempW = XMLoadFloat4x4( &obj2World );
	XMMATRIX &tempV = camera->getViewMatrix();
	XMMATRIX &tempP = camera->getProjectMatrix();
	XMMATRIX tempWVP = tempW * tempV * tempP;

	XMMATRIX identityMat = XMMatrixIdentity();

	efWVP->SetMatrix( reinterpret_cast<float*>( &tempWVP ) );
	efWorld->SetMatrix( reinterpret_cast<const float*>( &tempW ) );
	efTexTrans->SetMatrix( reinterpret_cast<const float*>( &identityMat ) );

	efMaterial->SetRawValue( &material , 0 , sizeof( CustomMaterial ) );
	efTexture->SetResource( textureView );
	efCameraPos->SetRawValue( &( camera->Position ) , 0 , sizeof( XMFLOAT3 ) );
}

void BasicQuad::RenderObject( ID3D11DeviceContext *immediateContext )
{
	immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST );
	immediateContext->IASetInputLayout( inputLayout );
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
