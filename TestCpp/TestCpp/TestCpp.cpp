// TestCpp.cpp : Defines the entry point for the console application.
//
#include <stdlib.h>
#include <iostream>
#include "TestIntegrator.h"

int main( int argc , char *argv[] )
{
	//TestIntegrator* ti = new TestIntegrator();
	//ti->Test4();

	std::string str;
	for ( int i = 1; i < argc; ++i )
	{
		str += argv[i];
	}
	
	std::cout << str << std::endl;

	system( "Pause" );
    return 0;
}


void print( int i )
{
	std::cout << i << std::endl;
}

void print( const int *a , size_t len )
{
	for ( size_t i = 0; i < len; ++i )
	{
		std::cout << a[i];
	}
	std::cout << std::endl;
}

void print( const int a[10] )
{
	for ( size_t i = 0; i < 10; ++i )
	{
		std::cout << a[i];
	}
	std::cout << std::endl;
}
