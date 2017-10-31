#pragma once
#include <DirectXMath.h>
#include <string>

struct ID3DX11Effect;
struct ID3DX11EffectTechnique;

class ShaderEffect
{
public:
	ShaderEffect();
	~ShaderEffect();

	void setShader( const std::string &shaderName , const std::string &techName );
	void createEffectAtRuntime( struct ID3D11Device *device );
	void createEffectAtBuildtime( struct ID3D11Device *device );

	ID3DX11EffectTechnique *getEffectTech();
	ID3DX11Effect *getEffect();

private:

	std::string shaderName;
	std::string techniqueName;
	struct ID3DX11Effect *effect;
};

