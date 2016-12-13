#include "MyTest.h"

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