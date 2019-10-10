#include<fstream>
#include "XXTEA.h"


#define MX ( z >> 5 ^ y << 2 ) + ( y >> 3 ^ z << 4 ) ^ ( sum^y ) + ( k[p & 3 ^ e] ^ z );

XXTEA::XXTEA()
{
	key = "chaofanshijie888";
}

void XXTEA::setFilePath( string newPath )
{
	filePath = newPath;
	readFromFile();
}

void XXTEA::EncryptFile()
{
	if ( data == NULL )return;

	int multi = sizeof( long ) / sizeof( char );

	btea( (long*) data , fileLen / multi , (long*) key.c_str() );
	saveToFile();
}

void XXTEA::DecryptFile()
{
	if ( data == NULL )return;

	int multi = sizeof( long ) / sizeof( char );

	btea( (long*) data , -fileLen / multi , (long*) key.c_str() );
	saveToFile();
}


XXTEA::~XXTEA()
{
	if ( data != NULL ) delete[] data;
}

int XXTEA::btea( long* v , long n , long* k )
{
	unsigned long z = 0 , y = v[0] , sum = 0 , e , DELTA = 0x9e3779b9;
	long p , q;
	if ( n > 1 )/* Coding Part */
	{
		q = 6 + 52 / n;
		while ( q-- > 0 )
		{
			sum += DELTA;
			e = ( sum >> 2 ) & 3;
			for ( p = 0; p < n - 1; p++ ) y = v[p + 1] , z = v[p] += MX;
			y = v[0];
			z = v[n - 1] += MX;
		}
		return 0;
	}
	else if ( n < -1 )/* Decoding Part */
	{
		n = -n;
		q = 6 + 52 / n;
		sum = q*DELTA;
		while ( sum != 0 )
		{
			e = ( sum >> 2 ) & 3;
			for ( p = n - 1; p > 0; p-- ) z = v[p - 1] , y = v[p] -= MX;
			z = v[n - 1];
			y = v[0] -= MX;
			sum -= DELTA;
		}
		return 0;
	}
	return 1;
}

void XXTEA::readFromFile()
{
	ifstream file( filePath.c_str() , ios::in | ios::binary );
	if ( file.good() )
	{
		file.seekg( 0 , ios::end );
		fileLen = (long) file.tellg();
		data = new char[fileLen];
		file.seekg( 0 , ios::beg );
		file.read( data , fileLen );
	}
	else
	{
		cout << "文件读取出错！！" << endl;
	}
	file.close();
}

void XXTEA::saveToFile()
{
	ofstream file( filePath.c_str() , ios::out | ios::binary );
	if ( file.good() )
	{
		file.write( data , fileLen );
	}
	else
	{
		cout << "文件写入出错！！" << endl;
	}
	file.close();
}

////////////////////MAIN//////////////////////
//int main( int argc , char* argv[] )
//{
//	cout << "欢迎使用XXTEA加解密工具" << endl;
//	XXTEA* tea = new XXTEA();
//
//	while ( true )
//	{
//		cout << "命令:e-加密 d-解密 q-退出" << endl;
//
//		char cmd[10];
//		cin >> cmd;
//		
//		if ( strcmp( cmd , "e" ) == 0 )
//		{
//			cout << "请拖入文件:" << endl;
//			char filePath[100];
//			cin >> filePath;
//
//			tea->setFilePath( filePath );
//			tea->EncryptFile();
//
//			cout << "加密完成！" << endl;
//		}
//		else if ( strcmp( cmd , "d" ) == 0 )
//		{
//			cout << "请拖入文件:" << endl;
//			char filePath[100];
//			cin >> filePath;
//
//			tea->setFilePath( filePath );
//			tea->DecryptFile();
//
//			cout << "解密完成！" << endl;
//		}
//		else if ( strcmp( cmd , "q" ) == 0 )
//		{
//			break;
//		}
//		else
//		{
//			cout << "命令错误！" << endl;
//		}
//	}
//	return 0;
//}