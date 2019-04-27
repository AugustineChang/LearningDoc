#include "Material.h"
#include "MyMath.h"
#include "Vector3.h"

bool Material::refract( const Vector3& in , const Vector3& normal , float niOverNt , Vector3 &refractRay )
{
	float cosIn = Vector3::dot( in , normal );
	float discriminant = 1.0f - ( niOverNt * niOverNt *( 1.0f - cosIn * cosIn ) );
	if ( discriminant > 0.0f )
	{
		refractRay = niOverNt * ( in - cosIn * normal ) - MyMath::squareRoot( discriminant )*normal;
		return true;
	}
	else
		return false;
}

Vector3 Material::reflect( const Vector3 &in , const Vector3 &normal )
{
	return in - 2.0f *Vector3::dot( in , normal ) * normal;
}