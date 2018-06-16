#pragma once
#include <mutex>
#include "Vector3.h"
#include "Ray.h"

class PPMImage;
class Scene;
class Camera;

class DrawTask
{
public:
	
	DrawTask( int fromRow , int toRow , int width , int height , int subPixel , std::mutex *mtx ,
		PPMImage *image , const Scene *simpleWorld , const Camera *camera , float *drawProgress );

	void operator()();

private:

	Vector3 getColor( const Ray &ray , int depth );

	int fromRow;
	int toRow;

	int width;
	int height;
	int subPixel;

	std::mutex *mtx;
	PPMImage *image;
	const Scene *simpleWorld;
	const Camera *camera;
	float *drawProgress;
};

