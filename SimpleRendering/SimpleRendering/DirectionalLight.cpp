#include "DirectionalLight.h"
#include "Matrix.h"

DirectionalLight::DirectionalLight( const Vector3 &eular )
{
	//setLightDir( eular );

	worldDir = eular;
}

void DirectionalLight::setLightDir( const Vector3 &eular )
{
	Vector4 lightDir = Vector4( 1.0f , 0.0f , 0.0f , 0.0f );
	lightDir = lightDir * Matrix::rotationMatrix( eular );

	worldDir = lightDir.getVector3();
	worldDir.normalized();
}
