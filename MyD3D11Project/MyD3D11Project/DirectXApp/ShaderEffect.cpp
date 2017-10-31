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


void ShaderEffect::setShader( const std::string &shader , const std::string &techName )
{
	shaderName = shader;
	techniqueName = techName;
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
}

void ShaderEffect::createEffectAtBuildtime( ID3D11Device *device )
{
	std::string path = std::string( "FX/" ) + shaderName + std::string( ".fxo" );
	std::ifstream fs( path.c_str() , std::ios::binary );
	assert( fs );

	fs.seekg( 0 , std::ios_base::end );
	size_t size = (size_t) fs.tellg();
	fs.seekg( 0 , std::ios_base::beg );
	std::vector<char> compiledShader( size );
	fs.read( &compiledShader[0] , size );
	fs.close();

	HR( D3DX11CreateEffectFromMemory( &compiledShader[0] , size , 0 , device , &effect ) );
}

ID3DX11EffectTechnique * ShaderEffect::getEffectTech()
{
	//"LightTech_Lit_Tex"
	return effect->GetTechniqueByName( techniqueName.c_str() );
}

ID3DX11Effect * ShaderEffect::getEffect()
{
	return effect;
}
