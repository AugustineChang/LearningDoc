#ifndef __XXTEA__
#define __XXTEA__
#include <iostream>
using namespace std;

class XXTEA
{
public:
	XXTEA();
	~XXTEA();
	void EncryptFile();
	void DecryptFile();
	void setFilePath( string newPath );

private:
	char* data;
	string key;
	long fileLen;
	string filePath;
	int btea( long* v , long n , long* k );
	void readFromFile();
	void saveToFile();
};

#endif