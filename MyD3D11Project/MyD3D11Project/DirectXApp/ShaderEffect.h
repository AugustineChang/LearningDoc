#pragma once
#include <DirectXMath.h>
#include <string>

struct ID3DX11Effect;
struct ID3DX11EffectTechnique;

class ShaderEffect
{
public:
	ShaderEffect( const std::string &shaderName );
	~ShaderEffect();

	void createEffectAtRuntime( struct ID3D11Device *device );
	void createEffectAtBuildtime( struct ID3D11Device *device );

	ID3DX11EffectTechnique *getEffectTech( const std::string &techName );
	ID3DX11Effect *getEffect();

private:

	std::string shaderName;
	struct ID3DX11Effect *effect;
};

