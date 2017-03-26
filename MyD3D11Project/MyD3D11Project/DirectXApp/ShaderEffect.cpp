#include "ShaderEffect.h"
#include "../Utilities/CommonHeader.h"
#include "../BasicShape/BasicShape.h"
#include "Camera.h"
#include "Lights.h"
#include <fstream>
#include <vector>
using namespace DirectX;

ShaderEffect::ShaderEffect()
{
}


ShaderEffect::~ShaderEffect()
{
	ReleaseCOM( effect );
}

void ShaderEffect::createEffectAtRuntime( ID3D11Device *device )
{
	DWORD shaderFlag = 0;
#if defined(DEBUG) | defined(_DEBUG)
	shaderFlag |= D3D10_SHADER_DEBUG;
	shaderFlag |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob *compilationMsgs;
	HRESULT hr = D3DX11CompileEffectFromFile( L"SimpleShader.fx" , 0 , 0 , shaderFlag , 0 , device , &effect , &compilationMsgs );

	// compilationMsgs can store errors or warnings.
	if ( compilationMsgs != 0 )
	{
		MessageBoxA( 0 , (char*) compilationMsgs->GetBufferPointer() , 0 , 0 );
		ReleaseCOM( compilationMsgs );
	}

	if ( FAILED( hr ) )
	{
		DXTrace( __FILEW__ , (DWORD) __LINE__ , hr ,
			L"D3DX11CompileFromFile" , true );
	}

	efWVP = effect->GetVariableByName( "gWVP" )->AsMatrix();
}

void ShaderEffect::createEffectAtBuildtime( ID3D11Device *device )
{
	std::ifstream fs( "FX/LitShader.fxo" , std::ios::binary );
	assert( fs );

	fs.seekg( 0 , std::ios_base::end );
	size_t size = (size_t) fs.tellg();
	fs.seekg( 0 , std::ios_base::beg );
	std::vector<char> compiledShader( size );
	fs.read( &compiledShader[0] , size );
	fs.close();

	HR( D3DX11CreateEffectFromMemory( &compiledShader[0] , size , 0 , device , &effect ) );
	efWVP = effect->GetVariableByName( "gWVP" )->AsMatrix();
	efWorld = effect->GetVariableByName( "gWorld" )->AsMatrix();
	efWorldNorm = effect->GetVariableByName( "gWorldNormal" )->AsMatrix();
	efTexTrans = effect->GetVariableByName( "gTexTransform" )->AsMatrix();

	efMaterial = effect->GetVariableByName( "gMaterial" );
	efTexture = effect->GetVariableByName( "diffuseTex" )->AsShaderResource();
	efAlphaTexture = effect->GetVariableByName( "diffuseAlphaTex" )->AsShaderResource();

	efDirLight = effect->GetVariableByName( "gDirectLight" );
	efPointLight = effect->GetVariableByName( "gPointLight" );
	efSpotLight = effect->GetVariableByName( "gSpotLight" );
	efCameraPos = effect->GetVariableByName( "gCameraPosW" )->AsVector();
}

void ShaderEffect::UpdateSceneEffect( Camera *camera , DirectionalLight *dirLight ,
	PointLight *pointLigiht , SpotLight *spotLigiht )
{
	efDirLight->SetRawValue( dirLight , 0 , sizeof( DirectionalLight ) );
	//efPointLight->SetRawValue( &pointLight , 0 , sizeof( PointLight ) );
	//efSpotLight->SetRawValue( &spotLight , 0 , sizeof( SpotLight ) );
	efCameraPos->SetRawValue( &( camera->Position ) , 0 , sizeof( XMFLOAT3 ) );
}

void ShaderEffect::UpdateObjectEffect( XMMATRIX &maxtrixWVP , XMMATRIX &toWorld ,
	XMMATRIX &normToWorld , XMMATRIX &texMatrix , const BasicShape *obj )
{
	efWVP->SetMatrix( reinterpret_cast<float*>( &maxtrixWVP ) );
	efWorld->SetMatrix( reinterpret_cast<const float*>( &toWorld ) );
	efWorldNorm->SetMatrix( reinterpret_cast<const float*>( &normToWorld ) );
	efTexTrans->SetMatrix( reinterpret_cast<const float*>( &texMatrix ) );

	efMaterial->SetRawValue( &( obj->getMaterial() ) , 0 , sizeof( CustomMaterial ) );

	efTexture->SetResource( obj->getTexture() );
	efAlphaTexture->SetResource( obj->getAlphaTexture() );
}

ID3DX11EffectTechnique * ShaderEffect::getEffectTech()
{
	if ( effectTech == nullptr )
	{
		effectTech = effect->GetTechniqueByName( "LightTech_Lit_Tex" );
	}

	return effectTech;
}
