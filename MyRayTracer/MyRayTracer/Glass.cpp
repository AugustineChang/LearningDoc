#include "Glass.h"
#include "MyMath.h"

Glass::Glass( float refra_index )
	: refractiveIndex( refra_index ) , albedo( Vector3( 1.0f , 1.0f , 1.0f ) )
{
}

Glass::Glass( const Vector3 &color , float refra_index  ) 
	: refractiveIndex( refra_index ) , albedo( color )
{
}

bool Glass::scatter( const Ray &ray_in , const HitResult& hitResult , Vector3 &attenuation , Ray &scatteredRay )
{
	float niOverNt;
	Vector3 realNormal;

	float cosRN = Vector3::dot( ray_in.getDirection() , hitResult.hitNormal );
	if ( cosRN > 0.0f ) //出射
	{
		niOverNt = refractiveIndex;
		realNormal = -hitResult.hitNormal;
		cosRN = MyMath::squareRoot( 1.0f - ( niOverNt * niOverNt *( 1.0f - cosRN * cosRN ) ) );
	}
	else //入射
	{
		niOverNt = 1.0f / refractiveIndex;
		realNormal = hitResult.hitNormal;
		cosRN = -cosRN;
	}

	Vector3 refractRay;
	float reflect_prob;
	if ( refract( ray_in.getDirection() , realNormal , niOverNt , refractRay ) )//折射
	{
		reflect_prob = schilick( cosRN );
	}
	else//全反射
	{
		reflect_prob = 1.0f;
	}

	//按概率决定 反射 还是 折射
	attenuation = albedo;
	if ( MyMath::getRandom01() < reflect_prob )
	{
		Vector3 reflectRay = reflect( ray_in.getDirection() , realNormal );
		reflectRay.normalized();
		scatteredRay = Ray( hitResult.hitPoint , reflectRay , ray_in.getSendTime() );
	}
	else
	{
		refractRay.normalized();
		scatteredRay = Ray( hitResult.hitPoint , refractRay , ray_in.getSendTime() );
	}
	return true;
}

float Glass::schilick( float cosine )//菲涅尔反射 概率
{
	float r0 = ( 1.0f - refractiveIndex ) / ( 1.0f + refractiveIndex );
	r0 = r0 * r0;
	return r0 + ( 1.0f - r0 )*MyMath::power( 1.0f - cosine , 5.0f );
}
