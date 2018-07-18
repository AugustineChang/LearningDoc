#include "ShapeRender.h"
#include "BaseShape.h"
#include "Camera.h"
#include "ScreenBuffer.h"
#include "SimpleMath.h"
#include "DirectionalLight.h"
#include "PPMImage.h"
#include <math.h>
#include <iostream>


ShapeRender::ShapeRender() : shape( nullptr ) , camera( nullptr ) , screen( nullptr ) ,
	cullState( 0 ) , ambientColor( 0.3f , 0.3f , 0.3f )
{
}

ShapeRender::ShapeRender( const Camera *cam , ScreenBuffer *scr )
	: camera( cam ) , screen( scr ) , cullState( 0 ) , ambientColor( 0.3f , 0.3f , 0.3f )
{
}

void ShapeRender::setRenderShape( const BaseShape *obj )
{
	if ( obj == nullptr || obj == shape )
		return;

	shape = obj;
}

void ShapeRender::setRenderScreen( ScreenBuffer *scr )
{
	if ( scr == nullptr || scr == screen )
		return;

	screen = scr;
}

void ShapeRender::setCullState( int cull )
{
	if ( cull < 0 || cull > 2 )return;

	cullState = cull;
}

void ShapeRender::setLight( const DirectionalLight *light )
{
	if ( light == mainLight ) return;

	mainLight = light;
}

void ShapeRender::setTexture( const PPMImage *tex )
{
	if ( tex == texture ) return;

	texture = tex;
}

void ShapeRender::renderShape()
{
	if ( shape == nullptr || camera == nullptr || screen == nullptr )
		return;

	//refresh imageSize
	screenHalfWidth = float( screen->getImageWidth() - 1 ) * 0.5f;
	screenHalfHeight = float( screen->getImageHeight() - 1 ) * 0.5f;

	//draw shape
	const Vertex* vertice;
	int numOfVertex;
	shape->getVertice( vertice , numOfVertex );
	const int* indice;
	int numOfIndex;
	shape->getIndice( indice , numOfIndex );

	Matrix viewMat = camera->getViewMatrix();
	Matrix clipMat = camera->getClipMatrix();
	Matrix worldMat = shape->getWorldMatrix();
	Matrix mv = worldMat * viewMat;
	Matrix mvp = mv * clipMat;

	Matrix normalWorld( worldMat );
	normalWorld.inverse();
	normalWorld.transpose();

	VertexInput inputData;
	inputData.mvp = &mvp;
	inputData.mv = &mv;
	inputData.world = &worldMat;
	inputData.normalWorld = &normalWorld;

	int numOfTri = numOfIndex / 3;
	for ( int i = 0; i < numOfTri; ++i )
	{
		int v0Index = indice[i * 3];
		int v1Index = indice[i * 3 + 1];
		int v2Index = indice[i * 3 + 2];

		drawTriangle( &vertice[v0Index] , &vertice[v1Index] , &vertice[v2Index] , inputData , shape->getDrawMode() );
	}
}

void ShapeRender::drawTriangle( const Vertex *v0 , const Vertex *v1 , const Vertex *v2 , const VertexInput &input , 
	int drawMode )
{
	//背面剔除
	if ( !backCullTest( v0 , v1 , v2 , input.mv ) ) return;

	VertexOutput v0_out = doVertexShader( v0 , input );
	VertexOutput v1_out = doVertexShader( v1 , input );
	VertexOutput v2_out = doVertexShader( v2 , input );

	//视锥体裁剪
	std::vector<VertexOutput> resultList;
	clipTriangle( v0_out , v1_out , v2_out , resultList );

	//重新生成三角形
	int numOfTri = SimpleMath::Max( int( resultList.size() ) - 2 , 0 );
	for ( int i = 1; i <= numOfTri; ++i )
	{
		VertexOutput out0 = resultList[0];
		VertexOutput out1 = resultList[i];
		VertexOutput out2 = resultList[i + 1];

		//投影除法
		projectDivision( out0 );
		projectDivision( out1 );
		projectDivision( out2 );

		if ( drawMode == 0 )
		{
			drawLine( out0 , out1 );
			drawLine( out1 , out2 );
			drawLine( out2 , out0 );
		}
		else if ( drawMode == 1 )
		{
			drawFillTriangle( &out0 , &out1 , &out2 );
		}
	}
}

VertexOutput ShapeRender::doVertexShader( const Vertex *v , const VertexInput &input )
{
	VertexOutput out;

	Vector4 curVertex( v->pos , 1.0f );
	Vector4 worldPos = curVertex * ( *input.world );

	Vector4 normalDir( v->normal , 0.0f );
	normalDir = normalDir * ( *input.normalWorld );

	out.pos = curVertex * ( *input.mvp );
	out.color = v->color;
	out.worldPos = worldPos.getVector3();
	out.normal = normalDir.getVector3();
	out.texUV = v->uvCoord;

	return out;
}

void ShapeRender::drawLine( const VertexOutput& start , const VertexOutput& end )
{
	int startX = SimpleMath::floorToInt( start.pos[0] );
	int startY = SimpleMath::floorToInt( start.pos[1] );
	int endX = SimpleMath::floorToInt( end.pos[0] );
	int endY = SimpleMath::floorToInt( end.pos[1] );

	startX = SimpleMath::Clamp( startX , 0 , screen->getImageWidth() );
	startY = SimpleMath::Clamp( startY , 0 , screen->getImageHeight() );
	endX = SimpleMath::Clamp( endX , 0 , screen->getImageWidth() );
	endY = SimpleMath::Clamp( endY , 0 , screen->getImageHeight() );

	//dx dy
	int dx = abs( endX - startX );
	int dy = abs( endY - startY );
	
	//统一正反划线
	int x , y , xStep , yStep;
	x = startX;
	y = startY;
	xStep = startX < endX ? 1 : -1;
	yStep = startY < endY ? 1 : -1;

	VertexOutput output;
	output.pos = start.pos;
	output.setData( start.normal , start.color , start.worldPos );

	//ZTest
	if ( screen->setDepth( x , y , output.pos[2] ) )
	{
		screen->setPixel( x , y , doFragmentShader( output ) );
	}

	//划线
	if ( dy <= dx )// k <= 1.0f
	{
		int two_dy = dy * 2;
		int two_dy_minus_dx = ( dy - dx ) * 2;
		int p = two_dy - dx;
		
		while ( x != endX )
		{
			x += xStep;
			if ( p < 0 )
			{
				p += two_dy;
			}
			else
			{
				y += yStep;
				p += two_dy_minus_dx;
			}
			
			float ratio = float( x - startX ) / float( endX - startX );
			output.pos = SimpleMath::Lerp( start.pos , end.pos , ratio );
			output.lerpData( start , end , ratio );

			//ZTest
			if ( !screen->setDepth( x , y , output.pos[2] ) )
				continue;

			screen->setPixel( x , y , doFragmentShader( output ) );
		}
	}
	else// k > 1.0f
	{
		int two_dx = dx * 2;
		int two_dx_minus_dy = ( dx - dy ) * 2;
		int p = two_dx - dy;
		
		while ( y != endY )
		{
			y += yStep;
			if ( p < 0 )
			{
				p += two_dx;
			}
			else
			{
				x += xStep;
				p += two_dx_minus_dy;
			}
			
			float ratio = float( y - startY ) / float( endY - startY );
			output.pos = SimpleMath::Lerp( start.pos , end.pos , ratio );
			output.lerpData( start , end , ratio );
			
			//ZTest
			if ( !screen->setDepth( x , y , output.pos[2] ) )
				continue;

			screen->setPixel( x , y , doFragmentShader( output ) );
		}
	}
}

void ShapeRender::drawFillTriangle( const VertexOutput *v0 , const VertexOutput *v1 , const VertexOutput *v2 )
{
	//三角形面积 (y1-y3)(x2-x3)+(y2-y3)(x3-x1)
	float triArea = ( v0->pos[1] - v2->pos[1] ) * ( v1->pos[0] - v2->pos[0] )
		+ ( v1->pos[1] - v2->pos[1] ) * ( v2->pos[0] - v0->pos[0] );

	if ( fabsf( triArea ) < 0.001f ) return;

	//上到下排序
	const VertexOutput *vList[3];
	float y0 , y1 , y2;
	y0 = v0->pos[1];
	y1 = v1->pos[1];
	y2 = v2->pos[1];

	if ( y0 > y1 )
	{
		vList[0] = v1;
		vList[1] = v0;
	}
	else
	{
		vList[0] = v0;
		vList[1] = v1;
	}

	if ( y2 < vList[0]->pos[1] )
	{
		vList[2] = vList[1];
		vList[1] = vList[0];
		vList[0] = v2;
	}
	else if ( y2 > vList[1]->pos[1] )
	{
		vList[2] = v2;
	}
	else
	{
		vList[2] = vList[1];
		vList[1] = v2;
	}

	//上到下 按行扫描
	int k0 , k1 , k2;
	k0 = SimpleMath::floorToInt( vList[0]->pos[1] );
	k1 = SimpleMath::floorToInt( vList[1]->pos[1] );
	k2 = SimpleMath::floorToInt( vList[2]->pos[1] );

	k0 = SimpleMath::Clamp( k0 , 0 , screen->getImageHeight() );
	k1 = SimpleMath::Clamp( k1 , 0 , screen->getImageHeight() );
	k2 = SimpleMath::Clamp( k2 , 0 , screen->getImageHeight() );

	for ( int i = k0; i <= k2; ++i )
	{
		if ( i <= k1 && k0 != k1 )
		{
			int start = getLineX( i , vList[0] , vList[1] );
			int end = getLineX( i , vList[0] , vList[2] );

			fillOneRow( i , start , end , v0 , v1 , v2 , triArea );
		}
		else
		{
			int start = getLineX( i , vList[1] , vList[2] );
			int end = getLineX( i , vList[0] , vList[2] );

			fillOneRow( i , start , end , v0 , v1 , v2 , triArea );
		}		
	}
}

Vector3 ShapeRender::doFragmentShader( const VertexOutput &v2f )
{
	Vector3 worldNormal = v2f.normal;
	worldNormal.normalized();
	
	Vector3 baseColor( v2f.color );
	if ( texture != nullptr )
	{
		Vector2 uv = v2f.texUV;
		uv.clamp01();
		baseColor = texture->sampleImage( uv );
	}

	float NdotL = 1.0f;
	if ( mainLight != nullptr )
	{
		NdotL = Vector3::dot( worldNormal , -mainLight->getLightDir() );
	}

	Vector3 diffuse = baseColor * SimpleMath::Max( NdotL , 0.0f );

	Vector3 ambient = baseColor * ambientColor;

	return diffuse + ambient;
}

bool ShapeRender::backCullTest( const Vertex *v0 , const Vertex *v1 , const Vertex *v2 , const Matrix *mv )
{
	if ( cullState == 0 )return true;

	Vector3 p0 = ( Vector4( v0->pos , 1.0f ) * ( *mv ) ).getVector3();
	Vector3 p1 = ( Vector4( v1->pos , 1.0f ) * ( *mv ) ).getVector3();
	Vector3 p2 = ( Vector4( v2->pos , 1.0f ) * ( *mv ) ).getVector3();

	Vector3 normal = Vector3::cross( p1 - p0 , p2 - p0 );
	float dotVal = Vector3::dot( p0 , normal );

	if ( cullState == 1 ) return dotVal < 0;
	else if ( cullState == 2 ) return dotVal > 0;
	
	return false;
}

void ShapeRender::clipTriangle( const VertexOutput &v0 , const VertexOutput &v1 , const VertexOutput &v2 , std::vector<VertexOutput> &outList )
{
	std::vector<VertexOutput> temp;
	std::vector<VertexOutput> oneClipIn;
	std::vector<VertexOutput> oneClipOut;
	oneClipIn.push_back( v0 );
	oneClipIn.push_back( v1 );
	oneClipIn.push_back( v2 );

	//依次进行裁剪 0-left 1-right 2-down 3-up 4-near 5-far
	for ( int state = 0; state < 6; ++state )
	{
		//某次裁剪 按顺序 两两端点执行裁剪
		int clipEdges = int( oneClipIn.size() );
		for ( int i = 0; i < clipEdges; ++i )
		{
			int next = ( i == clipEdges - 1 ) ? 0 : i + 1;
			clipEdge( oneClipIn[i] , oneClipIn[next] , state , temp );
			oneClipOut.insert( oneClipOut.end() , temp.begin() , temp.end() );
			temp.clear();
		}

		//结果是下次裁剪的输入
		oneClipIn = oneClipOut;
		oneClipOut.clear();
	}
	
	outList = oneClipIn;
}

void ShapeRender::projectDivision( VertexOutput &output )
{
	//换一下轴
	VertexOutput temp( output );
	output.pos[0] = temp.pos[1];
	output.pos[1] = temp.pos[2];
	output.pos[2] = temp.pos[0];
	output.pos[3] = temp.pos[3];

	//投影除法
	output.pos[0] = output.pos[0] / output.pos[3] * screenHalfWidth + screenHalfWidth;
	output.pos[1] = output.pos[1] / output.pos[3] * -screenHalfHeight + screenHalfHeight;
	output.pos[2] = output.pos[2] / output.pos[3];
}

void ShapeRender::fillOneRow( int k , int startX , int endX , const VertexOutput *v0 ,
	const VertexOutput *v1 , const VertexOutput *v2 , float triArea )
{
	//if ( abs( startX - endX ) <= 0 ) return;
	
	int x = startX;
	int lastX = x;
	int step = startX < endX ? 1 : -1;
	int times = abs( endX - startX ) + 1;
	
	VertexOutput output;
	for ( int i = 0; i < times; ++i )
	{
		Vector3 baryCoord = calcBaryCoord( x , k , v0->pos , v1->pos , v2->pos , triArea );
		output.pos = SimpleMath::BaryLerp( v0->pos , v1->pos , v2->pos , baryCoord );
		output.lerpData( *v0 , *v1 , *v2 , baryCoord );

		//ZTest
		if ( screen->setDepth( x , k , output.pos[2] ) )
		{
			screen->setPixel( x , k , doFragmentShader( output ) );
		}

		lastX = x;
		x += step;
	}
}

int ShapeRender::getLineX( int k , const VertexOutput *v0 , const VertexOutput *v1 )
{
	float x0 = v0->pos[0];
	float y0 = v0->pos[1];
	float x1 = v1->pos[0];
	float y1 = v1->pos[1];

	y0 = floorf( y0 );
	float ratio = ( float( k ) - y0 ) / ( y1 - y0 );
	return SimpleMath::floorToInt( x0 + ratio * ( x1 - x0 ) );
}

Vector3 ShapeRender::calcBaryCoord( int x , int y , const Vector4 &v0 , const Vector4 &v1 , const Vector4 &v2 , float triArea )
{
	//计算重心坐标
	float fy = float( y ) , fx = float( x );

	//(py-y3)(x2-x3)+(y2-y3)(x3-px)
	float b1 = ( fy - v2[1] ) * ( v1[0] - v2[0] )
		+ ( v1[1] - v2[1] ) * ( v2[0] - fx );
	b1 /= triArea;

	//(py-y1)(x3-x1)+(y3-y1)(x1-px)
	float b2 = ( fy - v0[1] ) * ( v2[0] - v0[0] )
		+ ( v2[1] - v0[1] ) * ( v0[0] - fx );
	b2 /= triArea;

	//(py-y2)(x1-x2)+(y1-y2)(x2-px)
	float b3 = ( fy - v1[1] ) * ( v0[0] - v1[0] )
		+ ( v0[1] - v1[1] ) * ( v1[0] - fx );
	b3 /= triArea;

	return Vector3( b1 , b2 , b3 );
}

void ShapeRender::clipEdge( const VertexOutput &v0 , const VertexOutput &v1 , int state , std::vector<VertexOutput> &outList )
{
	//裁剪第一点
	float t0 = 0.0f;
	VertexOutput newV0;
	newV0.pos = v0.pos;
	bool isFirstOut = isPointOut( newV0.pos , v0.pos , v1.pos , state , t0 );

	//裁剪第二点
	float t1 = 0.0f;
	VertexOutput newV1;
	newV1.pos = v1.pos;
	bool isSecondOut = isPointOut( newV1.pos , v0.pos , v1.pos , state , t1 );

	if ( isFirstOut && !isSecondOut )//外->内 v1',v2
	{
		newV0.lerpData( v0 , v1 , t0 , false );
		outList.push_back( newV0 );
		outList.push_back( v1 );
	}
	else if ( !isFirstOut && !isSecondOut )//内->内 v2
	{
		outList.push_back( v1 );
	}
	else if ( !isFirstOut && isSecondOut )//内->外 v2'
	{
		newV1.lerpData( v0 , v1 , t1 , false );
		outList.push_back( newV1 );
	}
}

bool ShapeRender::isPointOut( Vector4 &v , const Vector4 &start , const Vector4 &end , int state , float &outT )
{
	bool isOut = false;
	float w = v[3];

	switch ( state )
	{
	case 0://left
	{
		isOut = v[1] < -w;
		if ( isOut )
		{
			float den = ( start[1] + start[3] - end[1] - end[3] );
			float t = fabsf( den ) < 0.0001f ? 0.0f : ( start[1] + start[3] ) / den;
			outT = t;

			v = SimpleMath::Lerp( start , end , t );
		}
		break;
	}

	case 1://right
	{
		isOut = v[1] > w;
		if ( isOut )
		{
			float den = ( start[1] - start[3] - end[1] + end[3] );
			float t = fabsf( den ) < 0.0001f ? 0.0f : ( start[1] - start[3] ) / den;
			outT = t;

			v = SimpleMath::Lerp( start , end , t );
		}
		break;
	}

	case 2://down
	{
		isOut = v[2] < -w;
		if ( isOut )
		{
			float den = ( start[2] + start[3] - end[2] - end[3] );
			float t = fabsf( den ) < 0.0001f ? 0.0f : ( start[2] + start[3] ) / den;
			outT = t;

			v = SimpleMath::Lerp( start , end , t );
		}
		break;
	}

	case 3://up
	{
		isOut = v[2] > w;
		if ( isOut )
		{
			float den = ( start[2] - start[3] - end[2] + end[3] );
			float t = fabsf( den ) < 0.0001f ? 0.0f : ( start[2] - start[3] ) / den;
			outT = t;

			v = SimpleMath::Lerp( start , end , t );
		}
		break;
	}

	case 4://near
	{
		isOut = v[0] < 0.0f;
		if ( isOut )
		{
			float den = ( start[0] - end[0] );
			float t = fabsf( den ) < 0.0001f ? 0.0f : start[0] / den;
			outT = t;

			v[0] = 0.0f;
			v[1] = start[1] + ( end[1] - start[1] ) * t;
			v[2] = start[2] + ( end[2] - start[2] ) * t;
			v[3] = start[3] + ( end[3] - start[3] ) * t;
		}
		break;
	}

	case 5://far
	{
		isOut = v[0] > w;
		if ( isOut )
		{
			float den = ( start[0] - start[3] - end[0] + end[3] );
			float t = fabsf( den ) < 0.0001f ? 0.0f : ( start[0] - start[3] ) / den;
			outT = t;

			v = SimpleMath::Lerp( start , end , t );
		}
		break;
	}

	default:
		break;
	}

	return isOut;
}


//////////////////////////////////////////////VertexOutput//////////////////////////////////////
void VertexOutput::setData( const Vector3 &normal , const Vector3 &color , const Vector3 &worldPos )
{
	this->normal = normal;
	this->color = color;
	this->worldPos = worldPos;
}

void VertexOutput::lerpData( const VertexOutput &start , const VertexOutput &end , float alpha , bool isSS /*= true*/ )
{
	if ( isSS )
	{
		//透视矫正
		float inv_z1 = 1.0f / start.pos[3];
		float inv_z2 = 1.0f / end.pos[3];
		float inv_zt = SimpleMath::Lerp( inv_z1 , inv_z2 , alpha );
		//仅仅矫正uv
		this->texUV = SimpleMath::Lerp( start.texUV * inv_z1 , end.texUV * inv_z2 , alpha ) / inv_zt;
	}
	else
		this->texUV = SimpleMath::Lerp( start.texUV , end.texUV , alpha );

	this->normal = SimpleMath::Lerp( start.normal , end.normal , alpha );
	this->color = SimpleMath::Lerp( start.color , end.color , alpha );
	this->worldPos = SimpleMath::Lerp( start.worldPos , end.worldPos , alpha );
}

void VertexOutput::lerpData( const VertexOutput &v0 , const VertexOutput &v1 , const VertexOutput &v2 , const Vector3 &bary )
{
	//透视矫正
	float inv_z1 = 1.0f / v0.pos[3];
	float inv_z2 = 1.0f / v1.pos[3];
	float inv_z3 = 1.0f / v2.pos[3];
	float inv_zt = SimpleMath::BaryLerp( inv_z1 , inv_z2 , inv_z3 , bary );

	this->normal = SimpleMath::BaryLerp( v0.normal , v1.normal , v2.normal , bary );
	this->color = SimpleMath::BaryLerp( v0.color , v1.color , v2.color , bary );
	this->worldPos = SimpleMath::BaryLerp( v0.worldPos , v1.worldPos , v2.worldPos , bary );

	//仅仅矫正uv
	this->texUV = SimpleMath::BaryLerp( v0.texUV * inv_z1 , v1.texUV * inv_z2 , v2.texUV * inv_z3 , bary ) / inv_zt;
}
