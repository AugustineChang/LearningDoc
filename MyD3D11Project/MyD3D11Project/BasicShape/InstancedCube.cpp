#include "InstancedCube.h"
#include "../Utilities/CommonHeader.h"
#include "../DirectXApp/Lights.h"
#include "../DirectXApp/Camera.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
#include <sstream>
using namespace DirectX;


InstancedCube::InstancedCube() : BasicShape( "InstancedLitShader" ) , edgeNum( 3 ) , edgeLength( 10.0f )
{
	type = ShapeType::Instanced;
	techName = "InstanceTech_Lit_Tex_Fog";
}

InstancedCube::~InstancedCube()
{
}

void InstancedCube::UpdateObject( float DeltaTime , ID3D11DeviceContext *immediateContext )
{
	Rotation.y += 1.0f * DeltaTime;

	float twoPI = SimpleMath::PI * 2.0f;
	if ( Rotation.y > twoPI ) Rotation.y -= twoPI;

	buildWorldMatrix();

	////////////////////////////////////////////////////////////
	XMFLOAT3 center = XMFLOAT3( -0.5f*edgeLength , -0.5f*edgeLength , -0.5f*edgeLength );
	XMFLOAT3 delta = XMFLOAT3( edgeLength / ( edgeNum - 1 ) , edgeLength / ( edgeNum - 1 ) , edgeLength / ( edgeNum - 1 ) );

	for ( int k = 0; k < edgeNum; ++k )
	{
		for ( int i = 0; i < edgeNum; ++i )
		{
			for ( int j = 0; j < edgeNum; ++j )
			{
				int index = k *edgeNum*edgeNum + i*edgeNum + j;
				updateOneInstanceData( &instanceDataList[index] , center , delta , j , i , k );
			}
		}
	}

	/////////////////////////////////////////////////////////////
	D3D11_MAPPED_SUBRESOURCE mappedData;
	immediateContext->Map( instanceBuffer , 0 , D3D11_MAP_WRITE_DISCARD , 0 , &mappedData );

	InstanceData* dataView = reinterpret_cast<InstanceData*>( mappedData.pData );

	size_t len = instanceDataList.size();
	for ( size_t i = 0; i < len; ++i )
	{
		dataView[i].World = instanceDataList[i].World;
		dataView[i].Color = instanceDataList[i].Color;
	}

	immediateContext->Unmap( instanceBuffer , 0 );
}

void InstancedCube::InitShape( ID3D11Device *device )
{
	BasicShape::InitShape( device );
	createInstanceData( device );
}

void InstancedCube::createObjectMesh()
{
	XMFLOAT3 zero = XMFLOAT3( 0.0f , 0.0f , 0.0f );

	vertices =
	{
		{ XMFLOAT3( 1.0f,1.0f,1.0f ), zero , XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( -1.0f,1.0f,1.0f ), zero , XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( 1.0f,1.0f,-1.0f ), zero , XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f,1.0f,-1.0f ), zero , XMFLOAT2( 0.0f, 1.0f ) },//up

		{ XMFLOAT3( 1.0f,1.0f,1.0f ), zero , XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( -1.0f,1.0f,1.0f ), zero , XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( 1.0f,-1.0f,1.0f ), zero , XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f,-1.0f,1.0f ), zero , XMFLOAT2( 0.0f, 1.0f ) },//forward

		{ XMFLOAT3( 1.0f,1.0f,-1.0f ), zero , XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( -1.0f,1.0f,-1.0f ), zero , XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( 1.0f,-1.0f,-1.0f ), zero , XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f,-1.0f,-1.0f ), zero , XMFLOAT2( 0.0f, 1.0f ) },//back

		{ XMFLOAT3( -1.0f,1.0f,1.0f ), zero , XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( -1.0f,1.0f,-1.0f ), zero , XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( -1.0f,-1.0f,1.0f ), zero , XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f,-1.0f,-1.0f ), zero , XMFLOAT2( 0.0f, 1.0f ) },//left

		{ XMFLOAT3( 1.0f,1.0f,1.0f ), zero , XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( 1.0f,1.0f,-1.0f ), zero , XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( 1.0f,-1.0f,1.0f ), zero , XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( 1.0f,-1.0f,-1.0f ), zero , XMFLOAT2( 0.0f, 1.0f ) },//right

		{ XMFLOAT3( 1.0f,-1.0f,1.0f ), zero , XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( -1.0f,-1.0f,1.0f ), zero , XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( 1.0f,-1.0f,-1.0f ), zero , XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f,-1.0f,-1.0f ), zero , XMFLOAT2( 0.0f, 1.0f ) }//down
	};

	indices =
	{
		0, 2, 3,
		0, 3, 1, // up

		4, 7, 6,
		4, 5, 7, // forward

		8, 11, 9,
		8, 10, 11, // back

		12, 13, 14,
		13, 15, 14, // left

		16, 18, 19,
		16, 19, 17, // right

		20, 23, 22,
		20, 21, 23  // down
	};

	computeNormal();
}

void InstancedCube::createInputLayout( ID3D11Device *device )
{
	D3D11_INPUT_ELEMENT_DESC descList[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "WORLD",0,DXGI_FORMAT_R32G32B32A32_FLOAT,1,0,D3D11_INPUT_PER_INSTANCE_DATA,1 },
		{ "WORLD",1,DXGI_FORMAT_R32G32B32A32_FLOAT,1,16,D3D11_INPUT_PER_INSTANCE_DATA,1 },
		{ "WORLD",2,DXGI_FORMAT_R32G32B32A32_FLOAT,1,32,D3D11_INPUT_PER_INSTANCE_DATA,1 },
		{ "WORLD",3,DXGI_FORMAT_R32G32B32A32_FLOAT,1,48,D3D11_INPUT_PER_INSTANCE_DATA,1 },
		{ "COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,1,64,D3D11_INPUT_PER_INSTANCE_DATA,1 }
	};

	D3DX11_PASS_DESC passDesc;
	effect.getEffectTech( techName.c_str() )->GetPassByIndex( 0 )->GetDesc( &passDesc );

	HR( device->CreateInputLayout( descList , 8 , passDesc.pIAInputSignature , passDesc.IAInputSignatureSize , &inputLayout ) );
}

void InstancedCube::RenderObject( ID3D11DeviceContext *immediateContext )
{
	immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	immediateContext->IASetInputLayout( inputLayout );
	immediateContext->RSSetState( rasterState );

	float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	immediateContext->OMSetBlendState( blendState , blendFactors , 0xffffffff );

	ID3DX11EffectTechnique *technique = effect.getEffectTech( techName.c_str() );
	D3DX11_TECHNIQUE_DESC techDesc;
	technique->GetDesc( &techDesc );
	int drawNum = instanceDataList.size();
	for ( UINT i = 0; i < techDesc.Passes; ++i )
	{
		technique->GetPassByIndex( i )->Apply( 0 , immediateContext );

		immediateContext->DrawIndexedInstanced( indexSize , drawNum , indexStart , indexBase , 0 );
	}
}

void InstancedCube::UpdateObjectEffect( const Camera *camera )
{
	XMMATRIX &tempW = XMLoadFloat4x4( &obj2World );
	XMMATRIX &tempV = camera->getViewMatrix();
	XMMATRIX &tempP = camera->getProjectMatrix();
	XMMATRIX tempVP = tempV * tempP;

	XMMATRIX inverseW = XMMatrixInverse( &XMMatrixDeterminant( tempW ) , tempW );
	XMMATRIX inverseTransposeW = XMMatrixTranspose( inverseW );
	XMMATRIX identityMat = XMMatrixIdentity();

	efWVP->SetMatrix( reinterpret_cast<float*>( &tempVP ) );
	efWorld->SetMatrix( reinterpret_cast<const float*>( &tempW ) );
	efWorldNorm->SetMatrix( reinterpret_cast<const float*>( &inverseTransposeW ) );
	efTexTrans->SetMatrix( reinterpret_cast<const float*>( &identityMat ) );

	efMaterial->SetRawValue( material , 0 , sizeof( CustomMaterial ) );
	efTexture->SetResource( textureView );
	efCameraPos->SetRawValue( &( camera->Position ) , 0 , sizeof( XMFLOAT3 ) );

	if ( isEnableFog )
	{
		efFogStart->SetFloat( fogStart );
		efFogDistance->SetFloat( fogDistance );
		efFogColor->SetRawValue( &fogColor , 0 , sizeof( XMFLOAT4 ) );
	}
}

void InstancedCube::createRenderState( ID3D11Device *device )
{
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory( &rsDesc , sizeof( D3D11_RASTERIZER_DESC ) );
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;

	device->CreateRasterizerState( &rsDesc , &rasterState );
}

void InstancedCube::createObjectTexture( ID3D11Device *device )
{
	CreateDDSTextureFromFile( device , L"Textures/WoodCrate01.dds" , &texture , &textureView );
}

void InstancedCube::createInstanceData( ID3D11Device *device )
{
	instanceDataList.resize( edgeNum*edgeNum*edgeNum );

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof( InstanceData ) * instanceDataList.size();
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	HR( device->CreateBuffer( &bufferDesc , nullptr , &instanceBuffer ) );
}

void InstancedCube::updateOneInstanceData( InstanceData *data , const DirectX::XMFLOAT3 &center , 
	const DirectX::XMFLOAT3 &delta , int xIndex , int yIndex , int zIndex )
{
	float posX = center.x + xIndex * delta.x;
	float posY = center.y + yIndex * delta.y;
	float posZ = center.z + zIndex * delta.z;
	
	XMMATRIX &matrix = XMMatrixTranslation( posX , posY , posZ );
	XMMATRIX &toWorld = XMLoadFloat4x4( &obj2World );

	matrix = matrix * toWorld;
	XMStoreFloat4x4( &( data->World ) , matrix );

	data->Color.x = SimpleMath::RandF();
	data->Color.y = SimpleMath::RandF();
	data->Color.z = SimpleMath::RandF();
	data->Color.w = 1.0f;
}
