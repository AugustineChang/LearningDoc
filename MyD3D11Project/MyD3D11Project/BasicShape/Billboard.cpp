#include "Billboard.h"
#include "../Utilities/CommonHeader.h"
#include "../DirectXApp/Camera.h"
#include "DDSTextureLoader.h"
using namespace DirectX;

struct BillboardVertex
{
	DirectX::XMFLOAT3 CenterPos;
	DirectX::XMFLOAT2 Size;
};

Billboard::Billboard()
{
	type = ShapeType::Custom;
	isEnableFog = false;
	effect.setShader( "GeoShader" , "GeoTech_Tex" );
	isPassFrustumTest = true;
}


Billboard::~Billboard()
{
	ReleaseCOM( vertexBuffer );
	ReleaseCOM( indexBuffer );
}

void Billboard::InitShape( ID3D11Device *device )
{
	BasicShape::InitShape( device );
	createBuffers( device );
}

void Billboard::UpdateObjectEffect( const Camera *camera )
{
	buildWorldMatrix();

	XMMATRIX &tempW = XMLoadFloat4x4( &obj2World );
	XMMATRIX &tempV = camera->getViewMatrix();
	XMMATRIX &tempP = camera->getProjectMatrix();
	XMMATRIX tempVP = tempV * tempP;

	XMMATRIX identityMat = XMMatrixIdentity();

	efWVP->SetMatrix( reinterpret_cast<float*>( &tempVP ) );
	efWorld->SetMatrix( reinterpret_cast<float*>( &tempW ) );
	efTexTrans->SetMatrix( reinterpret_cast<const float*>( &identityMat ) );

	efTexture->SetResource( textureView );
	efCameraPos->SetRawValue( &( camera->Position ) , 0 , sizeof( XMFLOAT3 ) );

	XMFLOAT3 cameraUp;
	XMVECTOR up = camera->TransformDirection( XMVectorSet( 0.0f , 1.0f , 0.0f , 0.0f ) );
	XMStoreFloat3( &cameraUp , up );
	efCameraUp->SetRawValue( &cameraUp , 0 , sizeof( XMFLOAT3 ) );
}

void Billboard::RenderObject( ID3D11DeviceContext *immediateContext )
{
	immediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
	immediateContext->IASetInputLayout( inputLayout );
	immediateContext->RSSetState( rasterState );

	UINT stride = sizeof( BillboardVertex );
	UINT offset = 0;
	immediateContext->IASetVertexBuffers( 0 , 1 , &vertexBuffer , &stride , &offset );
	immediateContext->IASetIndexBuffer( indexBuffer , DXGI_FORMAT_R32_UINT , 0 );

	ID3DX11EffectTechnique *technique = effect.getEffectTech();
	D3DX11_TECHNIQUE_DESC techDesc;
	technique->GetDesc( &techDesc );
	for ( UINT i = 0; i < techDesc.Passes; ++i )
	{
		technique->GetPassByIndex( i )->Apply( 0 , immediateContext );

		immediateContext->DrawIndexed( 1 , 0 , 0 );
	}
}

void Billboard::createEffect( ID3D11Device *device )
{
	effect.createEffectAtBuildtime( device );

	efWVP = effect.getEffect()->GetVariableByName( "gVP" )->AsMatrix();
	efWorld = effect.getEffect()->GetVariableByName( "gWorld" )->AsMatrix();
	efTexTrans = effect.getEffect()->GetVariableByName( "gTexTransform" )->AsMatrix();

	efMaterial = effect.getEffect()->GetVariableByName( "gMaterial" );
	efTexture = effect.getEffect()->GetVariableByName( "diffuseTex" )->AsShaderResource();

	efCameraPos = effect.getEffect()->GetVariableByName( "gCameraPosW" )->AsVector();
	efCameraUp = effect.getEffect()->GetVariableByName( "gCameraUpW" )->AsVector();
}

void Billboard::createInputLayout( ID3D11Device *device )
{
	D3D11_INPUT_ELEMENT_DESC descList[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "SIZE",0,DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};

	D3DX11_PASS_DESC passDesc;
	effect.getEffectTech()->GetPassByIndex( 0 )->GetDesc( &passDesc );

	HR( device->CreateInputLayout( descList , 2 , passDesc.pIAInputSignature , passDesc.IAInputSignatureSize , &inputLayout ) );
}

void Billboard::createObjectTexture( ID3D11Device *device )
{
	CreateDDSTextureFromFile( device , L"Textures/flare.dds" , &texture , &textureView );
}

void Billboard::createBuffers( ID3D11Device *device )
{
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof( BillboardVertex );
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	BillboardVertex vertex;
	vertex.CenterPos = XMFLOAT3( 0.0f , 0.0f , 0.0f );
	vertex.Size = XMFLOAT2( 1.0f , 1.0f );

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &vertex;

	HR( device->CreateBuffer( &bufferDesc , &initData , &vertexBuffer ) );

	D3D11_BUFFER_DESC bufferDesc2;
	bufferDesc2.ByteWidth = sizeof( UINT );
	bufferDesc2.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc2.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc2.CPUAccessFlags = 0;
	bufferDesc2.MiscFlags = 0;
	bufferDesc2.StructureByteStride = 0;

	UINT index = 0;
	D3D11_SUBRESOURCE_DATA initData2;
	initData2.pSysMem = &index;

	HR( device->CreateBuffer( &bufferDesc2 , &initData2 , &indexBuffer ) );
}
