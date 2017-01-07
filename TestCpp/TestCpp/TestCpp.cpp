// TestCpp.cpp : Defines the entry point for the console application.
//
#include <stdlib.h>
#include <iostream>
#include "TestIntegrator.h"
#include "BitArray.h"
#include "MyTest.h"
#include "TextQuery.h"

void print( int i );

int main( int argc , const char *argv )
{
	TestIntegrator* ti = new TestIntegrator();
	ti->Test6( 5 );
	
	/*BitArray bitArray;
	bitArray.setValue( 14 , true );
	bitArray.setValue( 1 , true );

	int len = bitArray.getMaxLen();
	for ( int i = 0; i < len; i++ )
	{
		std::cout << bitArray.getValue( i ) << ' ';
	}
	std::cout << std::endl;*/
	
	/*std::string str;
	for ( int i = 1; i < argc; ++i )
	{
		str += argv[i];
	}
	
	std::cout << str << std::endl;*/

	TextQuery query;
	query.readFromStream();
	TextQueryResult result = query.getQueryResult( "is" );
	result.showQueryResult();

	MyTest test;
	test.setNum( 11 );

	MyTest test2;
	test2.setNum( 22 );

	MyTest test3 = addNum( test , test2 );

	print( test.callFunc( &test2 , &MyTest::getNum ) );

	std::cout << test3.getNum() << std::endl;

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
