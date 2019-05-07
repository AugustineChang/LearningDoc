#include "Glass.h"
#include "MyMath.h"
#include "Ray.h"
#include "Hitable.h"
#include "Texture.h"

Glass::Glass( float refra_index )
	: refractiveIndex( refra_index ) , albedo( nullptr )
{
}

Glass::Glass( const Texture *tex , float refra_index  )
	: refractiveIndex( refra_index ) , albedo( tex )
{
}

bool Glass::scatter( const Ray &ray_in , const HitResult& hitResult , Vector3 &attenuation , Ray &scatteredRay )
{
	float niOverNt;
	Vector3 realNormal;

	float cosRN = Vector3::dot( ray_in.getDirection() , hitResult.hitNormal );
	if ( cosRN > 0.0f ) //����
	{
		niOverNt = refractiveIndex;
		realNormal = -hitResult.hitNormal;
		cosRN = MyMath::squareRoot( 1.0f - ( niOverNt * niOverNt *( 1.0f - cosRN * cosRN ) ) );
	}
	else //����
	{
		niOverNt = 1.0f / refractiveIndex;
		realNormal = hitResult.hitNormal;
		cosRN = -cosRN;
	}

	Vector3 refractRay;
	float reflect_prob;
	if ( refract( ray_in.getDirection() , realNormal , niOverNt , refractRay ) )//����
	{
		reflect_prob = schilick( cosRN );
	}
	else//ȫ����
	{
		reflect_prob = 1.0f;
	}
	
	if ( albedo == nullptr )
		attenuation = Vector3::oneVector;
	else
		attenuation = albedo->sample( hitResult.hitUVCoord[0] , hitResult.hitUVCoord[1] , hitResult.hitPoint );

	//�����ʾ��� ���� ���� ����
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

float Glass::schilick( float cosine )//���������� ����
{
	float r0 = ( 1.0f - refractiveIndex ) / ( 1.0f + refractiveIndex );
	r0 = r0 * r0;
	return r0 + ( 1.0f - r0 )*MyMath::power( 1.0f - cosine , 5.0f );
}
