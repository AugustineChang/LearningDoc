#include "MyTest.h"
#include <iostream>

int MyTest::a = 0;

MyTest::MyTest(): number(0)
{
}


MyTest::~MyTest()
{
}


int MyTest::getNum() const
{
	return number;
}

void MyTest::setNum( int val )
{
	number = val;
}


MyTest addNum( const MyTest &A , const MyTest &B )
{
	MyTest sum;
	sum.setNum( A.number + B.number );
	return sum;
}

int MyTest::callFunc( MyTest *obj,  myDelegate func )
{
	return (obj->*func)();
}


std::vector<int>* MyTest::getVectorPtr()
{
	return new std::vector<int>();
}

void MyTest::saveInput( std::vector<int>* ptr )
{
	int input;
	while ( std::cin >> input )
	{
		ptr->push_back( input );
	}
}

void MyTest::outputVector( std::vector<int>* ptr )
{
	for ( auto i = ptr->begin(); i != ptr->end(); ++i )
	{
		std::cout << *i << ' ';
	}
	std::cout << std::endl;
}