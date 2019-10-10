// TestCpp.cpp : Defines the entry point for the console application.
//
#include <stdlib.h>
#include <iostream>
#include <memory>
#include "TestIntegrator.h"
#include "BitArray.h"
#include "MyTest.h"
#include "TextQuery.h"
#include "TextQueryResult.h"
#include "Folder.h"
#include "Message.h"

void print( int i );

void testFolderMessage();

int myMain( int argc , const char *argv )
{
	/*TestIntegrator* ti = new TestIntegrator();
	ti->Test6( 5 );*/
	
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

	/*TextQuery query;
	query.readFromStream();
	TextQueryResult result = query.getQueryResult( "is" );
	result.showQueryResult();

	MyTest test;
	test.setNum( 11 );

	MyTest test2;
	test2.setNum( 22 );

	MyTest test3 = addNum( test , test2 );

	print( test.callFunc( &test2 , &MyTest::getNum ) );

	std::cout << test3.getNum() << std::endl;*/

	/*std::string str = "aaa";
	std::vector<std::string> testVector;
	testVector.push_back( str );
	str.append( "bbb" );
	std::cout << str << std::endl;
	std::cout << testVector[0] << std::endl;*/

	//auto ptr = test.getVectorPtr();
	//test.saveInput( ptr );
	//test.outputVector( ptr );
	//delete ptr;

	//testFolderMessage();

	const int a = 10;
	const int *b = &a;
	int *c = const_cast<int*>( b );

	std::cout << a << " " << *b << " " << *c << std::endl;
	*c = 20;
	std::cout << a << " " << *b << " " << *c << std::endl;

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


void testFolderMessage()
{
	Message M1( "MessageA" );
	Message M2( "MessageB" );
	Message M3( "MessageC" );

	Folder F1;
	Folder F2;
	
	F1.AddToFolder( &M1 );
	F1.AddToFolder( &M2 );
	F1.AddToFolder( &M3 );
	F2.AddToFolder( &M3 );

	std::cout << "F1" << std::endl;
	F1.ShowAllMessage();

	std::cout << "F2" << std::endl;
	F2.ShowAllMessage();
	
	
	{
		Folder F3( F1 );
		M1.SetMessage( "MessageA//Change" );

		std::cout << "F3" << std::endl;
		F3.ShowAllMessage();

		M3.showReferenceCount();
	}
	

	std::cout << "F1" << std::endl;
	F1.ShowAllMessage();

	std::cout << "F2" << std::endl;
	F2.ShowAllMessage();

	M3.showReferenceCount();
}