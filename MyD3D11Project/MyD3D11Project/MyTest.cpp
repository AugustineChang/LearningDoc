#include <windows.h>
#include <DirectXMath.h>
#include <iostream>
#include <iomanip>
using namespace std;
using namespace DirectX;

ostream& operator<<( ostream &stream , FXMVECTOR vec )
{
	XMFLOAT4 float4;
	DirectX::XMStoreFloat4( &float4 , vec );

	stream << "(" << float4.x << "," << float4.y << "," << float4.z << "," << float4.w << ")";
	return stream;
}

ostream& operator<< ( ostream &stream , CXMMATRIX matrix )
{
	XMFLOAT4X4 float4x4;
	XMStoreFloat4x4( &float4x4 , matrix );

	for ( size_t i = 0; i < 4; ++i )
	{
		stream << "|";
		for ( size_t j = 0; j < 4; ++j )
		{
			stream << float4x4( i , j ) << " ";
		}
		stream << "|" << endl;
	}

	return stream;
}

int main()
{
	if ( !XMVerifyCPUSupport() )
	{
		cout << "xna math library is not Supported!!" << endl;
		return 0;
	}

	XMMATRIX matrixA = XMMatrixSet( 1.0f , 2.0f , 0.0f , 0.0f ,
		3.0f , 4.0f , 0.0f , 0.0f ,
		0.0f , 0.0f , 1.0f , 0.0f ,
		0.0f , 0.0f , 0.0f , 1.0f );
	cout << matrixA << endl;
	cout << XMMatrixInverse( &XMMatrixDeterminant( matrixA ) , matrixA );

	system( "pause" );
	return 0;
}

void testAboutVector()
{
	XMVECTOR vecV = XMVectorSet( 3.0f , -1.7f , 0.5f , 0.0f );
	XMVECTOR vecU = XMVectorSet( -2.1f , 3.2f , -1.5f , 0.0f );

	cout << "v = " << vecV << endl;
	cout << "u = " << vecU << endl;
	cout << "v + u = " << vecV + vecU << endl;
	cout << "v * u = " << vecV * vecU << endl;
	cout << "v dot u = " << XMVector3Dot( vecV , vecU ) << endl;

	XMVECTOR radius = XMVector3AngleBetweenVectors( vecV , vecU );
	float angleDegree = XMConvertToDegrees( XMVectorGetX( radius ) );
	cout << "angle between v and u = " << angleDegree << endl;

	XMVECTOR vecOne = XMVectorSet( 1.0f , 1.0f , 1.0f , 0.0f );
	XMVECTOR vecN = XMVector3Normalize( vecOne );
	float length = XMVectorGetX( XMVector3Length( vecN ) );
	cout << "length of VecOne = " << setiosflags( ios::fixed ) << setprecision( 10 ) << length << endl;
	if ( length == 1.0f ) cout << "length is 1.0f" << endl;
	else cout << "length is not 1.0f" << endl;
	cout << "1 pow 1.0x10^6 = " << powf( length , 1.0e6f ) << endl;
}

void testAboutMatrix()
{
	XMMATRIX matrix_A( 1.0f , 0.0f , 0.0f , 0.0f ,
		0.0f , 2.0f , 0.0f , 0.0f ,
		0.0f , 0.0f , 4.0f , 0.0f ,
		1.0f , 2.0f , 3.0f , 1.0f );

	XMMATRIX matrix_B = XMMatrixIdentity();
	cout << "Matrix A = \n" << matrix_A << endl;
	cout << "Matrix B = \n" << matrix_B << endl;
	cout << "Matrix A*B = \n" << matrix_A * matrix_B << endl;
	cout << "Matrix A^t = \n" << XMMatrixTranspose( matrix_A ) << endl;
	cout << "Matrix A^-1 = \n" << XMMatrixInverse( &XMMatrixDeterminant( matrix_A ) , matrix_A ) << endl;
}

void orthogonalizeVectors( XMVECTOR &v0 , XMVECTOR &v1 , XMVECTOR &v2 )
{
	v0 = XMVector3Normalize( v0 );
	v2 = XMVector3Normalize( XMVector3Cross( v0 , v1 ) );
	v1 = XMVector3Cross( v0 , v2 );
}