#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <vector>

class BaseShape;
class Camera;
class ScreenBuffer;
struct Vertex;
class Matrix;
class DirectionalLight;
class PPMImage;

struct VertexInput
{
	const Matrix *mvp;
	const Matrix *mv;
	const Matrix *world;
	const Matrix *normalWorld;
};

struct VertexOutput
{
	Vector4 pos;
	Vector3 normal;
	Vector3 color;
	Vector3 worldPos;
	Vector2 texUV;

	void setData( const Vector3 &normal , const Vector3 &color , const Vector3 &worldPos );

	void lerpData( const VertexOutput &start , const VertexOutput &end , float alpha , bool isSS = true );
	void lerpData( const VertexOutput &v0 , const VertexOutput &v1 , const VertexOutput &v2 , const Vector3 &bary );
};

class ShapeRender
{
public:

	ShapeRender();
	ShapeRender( const Camera *cam , ScreenBuffer *img );

	void setRenderShape( const BaseShape *obj );
	void setRenderScreen( ScreenBuffer *scr );
	void setCullState( int cull );
	void setLight( const DirectionalLight *light );
	void setTexture( const PPMImage *tex );

	void renderShape();

private:

	void drawTriangle( const Vertex *v0 , const Vertex *v1 , const Vertex *v2 , const VertexInput &input , int drawMode );
	VertexOutput doVertexShader( const Vertex *v , const VertexInput &input );
	
	void drawLine( const VertexOutput& start , const VertexOutput& end );
	void drawFillTriangle( const VertexOutput *v0 , const VertexOutput *v1 , const VertexOutput *v2 );
	Vector3 doFragmentShader( const VertexOutput &v2f );

	bool backCullTest( const Vertex *v0 , const Vertex *v1 , const Vertex *v2 , const Matrix *mv );
	void clipTriangle( const VertexOutput &v0 , const VertexOutput &v1 , const VertexOutput &v2 , 
		std::vector<VertexOutput> &outList );
	void projectDivision( VertexOutput &output );

private:

	void fillOneRow( int k , int startX , int endX , const VertexOutput *v0 , const VertexOutput *v1 ,
		const VertexOutput *v2 , float triArea );
	int getLineX( int k , const VertexOutput *v0 , const VertexOutput *v1 );
	Vector3 calcBaryCoord( int x , int y , const Vector4 &v0 , const Vector4 &v1 , const Vector4 &v2 , float triArea );
	void clipEdge( const VertexOutput &v0 , const VertexOutput &v1 , int state , std::vector<VertexOutput> &outList );
	bool isPointOut( Vector4 &v , const Vector4 &start , const Vector4 &end , int state , float &outT );

private:
	
	Vector3 ambientColor;

	const BaseShape *shape;
	const Camera *camera;
	ScreenBuffer *screen;
	const DirectionalLight *mainLight;
	const PPMImage *texture;

	float screenHalfWidth;
	float screenHalfHeight;
	int cullState;//0-none 1-backCull 2-frontCull
};

