#pragma once

class Ray;
class Vector3;
struct HitResult;

class Material
{
public:
	virtual bool scatter( const Ray &ray_in , const HitResult& hitResult , Vector3 &attenuation , Ray &scatteredRay ) = 0;

protected:

	bool refract( const Vector3& in , const Vector3& normal , float niOverNt , Vector3 &refractRay );
	
	Vector3 reflect( const Vector3 &in , const Vector3 &normal );
};