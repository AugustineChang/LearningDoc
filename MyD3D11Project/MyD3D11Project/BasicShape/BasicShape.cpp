#include "BasicShape.h"
#include "../Utilities/CommonHeader.h"
#include "../DirectXApp/Camera.h"
#include "../DirectXApp/Lights.h"
using namespace DirectX;

BasicShape::BasicShape() : effect( "LitShader" ) , isUseGlobalBuffer( true ) , isEnableFog( true ) , 
fogStart( 20.0f ) , fogDistance( 100.0f ) , fogColor( XMFLOAT4( 0.5921f , 0.7412f , 0.7686f , 1.0f ) )
{
	initDirectMath();
	techName = isEnableFog ? "LightTech_Lit_Tex_Fog" : "LightTech_Lit_Tex";
}

BasicShape::BasicShape( std::string shader ) : effect( shader ) , isUseGlobalBuffer( true ) , isEnableFog( true ) ,
fogStart( 20.0f ) , fogDistance( 100.0f ) , fogColor( XMFLOAT4( 0.5921f , 0.7412f , 0.7686f , 1.0f ) )
{
	initDirectMath();
	techName = isEnableFog ? "LightTech_Lit_Tex_Fog" : "LightTech_Lit_Tex";
}

BasicShape::~BasicShape()
{
	ReleaseCOM( textureView );
	ReleaseCOM( texture );
	ReleaseCOM( inputLayout );
	ReleaseCOM( blendState );
	ReleaseCOM( rasterState );
}

void BasicShape::initDirectMath()
{
	XMMATRIX identityMaxtrix = DirectX::XMMatrixIdentity();
	XMStoreFloat4x4( &obj2World , identityMaxtrix );

	XMVECTOR objPos = XMVectorSet( 0.0f , 0.0f , 0.0f , 1.0f );
	XMStoreFloat4( &Position , objPos );

	XMVECTOR objRot = XMVectorSet( 0.0f , 0.0f , 0.0f , 0.0f );
	XMStoreFloat3( &Rotation , objRot );

	XMVECTOR objScale = XMVectorSet( 1.0f , 1.0f , 1.0f , 0.0f );
	XMStoreFloat3( &Scale , objScale );

	material.ambient = XMFLOAT4( 1.0f , 1.0f , 1.0f , 1.0f );
	material.diffuse = XMFLOAT4( 1.0f , 1.0f , 1.0f , 1.0f );
	material.specular = XMFLOAT4( 1.0f , 1.0f , 1.0f , 5.0f );
}

void BasicShape::buildWorldMatrix( )
{
	FXMVECTOR scale = XMLoadFloat3( &Scale );
	FXMVECTOR rotation = XMLoadFloat3( &Rotation );
	FXMVECTOR position = XMLoadFloat4( &Position );

	XMMATRIX scaleMat = DirectX::XMMatrixScalingFromVector( scale );
	XMMATRIX rotationMat = DirectX::XMMatrixRotationRollPitchYawFromVector( rotation );
	XMMATRIX transformMat = DirectX::XMMatrixTranslationFromVector( position );

	XMMATRIX temp = scaleMat * rotationMat * transformMat;
	XMStoreFloat4x4( &obj2World , temp );
}

void BasicShape::InitShape( struct ID3D11Device *device )
{
	createObjectMesh();
	createEffect( device );
	createInputLayout( device );
	createObjectTexture( device );
	createBlendState( device );
	createRenderState( device );
}

void BasicShape::UpdateObjectEffect( const Camera *camera )
{
	buildWorldMatrix();

	XMMATRIX &tempW = XMLoadFloat4x4( &obj2World );
	XMMATRIX &tempV = camera->getViewMatrix();
	XMMATRIX &tempP = camera->getProjectMatrix();
	XMMATRIX tempWVP = tempW * tempV * tempP;

	XMMATRIX inverseW = XMMatrixInverse( &XMMatrixDeterminant( tempW ) , tempW );
	XMMATRIX inverseTransposeW = XMMatrixTranspose( inverseW );
	XMMATRIX identityMat = XMMatrixIdentity();

	efWVP->SetMatrix( reinterpret_cast<float*>( &tempWVP ) );
	efWorld->SetMatrix( reinterpret_cast<const float*>( &tempW ) );
	efWorldNorm->SetMatrix( reinterpret_cast<const float*>( &inverseTransposeW ) );
	efTexTrans->SetMatrix( reinterpret_cast<const float*>( &identityMat ) );

	efMaterial->SetRawValue( &material , 0 , sizeof( CustomMaterial ) );
	efTexture->SetResource( textureView );
	efCameraPos->SetRawValue( &( camera->Position ) , 0 , sizeof( XMFLOAT3 ) );

	if ( isEnableFog )
	{
		efFogStart->SetFloat( fogStart );
		efFogDistance->SetFloat( fogDistance );
		efFogColor->SetRawValue( &fogColor , 0 , sizeof( XMFLOAT4 ) );
	}
}

void BasicShape::UpdateDirectionalLight( const DirectionalLight *dirLight , int lightNum )
{
	if ( lightNum < 0 || lightNum > 3 ) return;

	efDirLight->SetRawValue( dirLight , 0 , lightNum * sizeof( DirectionalLight ) );
	efDirLightNum->SetInt( lightNum );
}

void BasicShape::UpdatePointLight( const PointLight *pointLight , int lightNum )
{
	if ( lightNum < 0 || lightNum > 3 ) return;

	efPointLight->SetRawValue( pointLight , 0 , lightNum * sizeof( PointLight ) );
	efPointLightNum->SetInt( lightNum );
}

void BasicShape::UpdateSpotLight( const SpotLight *spotLight , int lightNum )
{
	if ( lightNum < 0 || lightNum > 3 ) return;

	efSpotLight->SetRawValue( spotLight , 0 , lightNum * sizeof( SpotLight ) );
	efSpotLightNum->SetInt( lightNum );
}

void BasicShape::RenderObject( ID3D11DeviceContext *immediateContext )
{
	immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	immediateContext->IASetInputLayout( inputLayout );
	immediateContext->RSSetState( rasterState );

	float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	immediateContext->OMSetBlendState( blendState , blendFactors , 0xffffffff );

	ID3DX11EffectTechnique *technique = effect.getEffectTech( techName.c_str() );
	D3DX11_TECHNIQUE_DESC techDesc;
	technique->GetDesc( &techDesc );
	for ( UINT i = 0; i < techDesc.Passes; ++i )
	{
		technique->GetPassByIndex( i )->Apply( 0 , immediateContext );

		immediateContext->DrawIndexed( indexSize , indexStart , indexBase );
	}
}

void BasicShape::createEffect( ID3D11Device *device )
{
	effect.createEffectAtBuildtime( device );

	efWVP = effect.getEffect()->GetVariableByName( "gWVP" )->AsMatrix();
	efWorld = effect.getEffect()->GetVariableByName( "gWorld" )->AsMatrix();
	efWorldNorm = effect.getEffect()->GetVariableByName( "gWorldNormal" )->AsMatrix();
	efTexTrans = effect.getEffect()->GetVariableByName( "gTexTransform" )->AsMatrix();

	efMaterial = effect.getEffect()->GetVariableByName( "gMaterial" );
	efTexture = effect.getEffect()->GetVariableByName( "diffuseTex" )->AsShaderResource();
	
	efDirLight = effect.getEffect()->GetVariableByName( "gDirectLight" );
	efPointLight = effect.getEffect()->GetVariableByName( "gPointLight" );
	efSpotLight = effect.getEffect()->GetVariableByName( "gSpotLight" );
	efDirLightNum = effect.getEffect()->GetVariableByName( "gDirLightNum" )->AsScalar();
	efPointLightNum = effect.getEffect()->GetVariableByName( "gPointLightNum" )->AsScalar();
	efSpotLightNum = effect.getEffect()->GetVariableByName( "gSpotLightNum" )->AsScalar();

	efCameraPos = effect.getEffect()->GetVariableByName( "gCameraPosW" )->AsVector();

	if ( isEnableFog )
	{
		efFogStart = effect.getEffect()->GetVariableByName( "gFogStart" )->AsScalar();
		efFogDistance = effect.getEffect()->GetVariableByName( "gFogDistance" )->AsScalar();
		efFogColor = effect.getEffect()->GetVariableByName( "gFogColor" )->AsVector();
	}
}

void BasicShape::createInputLayout( ID3D11Device *device )
{
	D3D11_INPUT_ELEMENT_DESC descList[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};

	D3DX11_PASS_DESC passDesc;
	effect.getEffectTech( techName.c_str() )->GetPassByIndex( 0 )->GetDesc( &passDesc );

	HR( device->CreateInputLayout( descList , 3 , passDesc.pIAInputSignature , passDesc.IAInputSignatureSize , &inputLayout ) );
}

void BasicShape::createRenderState( ID3D11Device *device )
{
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory( &rsDesc , sizeof( D3D11_RASTERIZER_DESC ) );
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;

	device->CreateRasterizerState( &rsDesc , &rasterState );
}

void BasicShape::computeNormal()
{
	unsigned int indexNum = indices.size();
	unsigned int triangleNum = indexNum / 3;
	for ( unsigned int i = 0; i < triangleNum; ++i )
	{
		unsigned int i0 = indices[i * 3];
		unsigned int i1 = indices[i * 3 + 1];
		unsigned int i2 = indices[i * 3 + 2];

		XMVECTOR v0 = XMLoadFloat3( &vertices[i0].Pos );
		XMVECTOR v1 = XMLoadFloat3( &vertices[i1].Pos );
		XMVECTOR v2 = XMLoadFloat3( &vertices[i2].Pos );

		XMVECTOR u = v1 - v0;
		XMVECTOR v = v2 - v0;
		XMVECTOR normal = XMVector3Cross( u , v );

		XMVECTOR originNor = XMLoadFloat3( &vertices[i0].Normal );
		XMStoreFloat3( &vertices[i0].Normal , originNor + normal );
		originNor = XMLoadFloat3( &vertices[i1].Normal );
		XMStoreFloat3( &vertices[i1].Normal , originNor + normal );
		originNor = XMLoadFloat3( &vertices[i2].Normal );
		XMStoreFloat3( &vertices[i2].Normal , originNor + normal );
	}

	unsigned int vertexNum = vertices.size();
	for ( unsigned int i = 0; i < vertexNum; ++i )
	{
		XMVECTOR normal = XMLoadFloat3( &vertices[i].Normal );
		normal = XMVector3Normalize( normal );
		XMStoreFloat3( &vertices[i].Normal , normal );
	}
		
}
