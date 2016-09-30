#include "TestIntegrator.h"
#include <iostream>
using namespace std;

TestIntegrator::TestIntegrator()
{
}


TestIntegrator::~TestIntegrator()
{
}

void TestIntegrator::Test1()
{
	cout << "a=?,b=?" << endl;
	int a = 0 , b = 0;
	cin >> a >> b;

	cout << "a+b=" << ( a + b ) << endl;
}

void TestIntegrator::Test2()
{
	int counter = 1;
	int sum = 0;
	while ( counter <= 10 )
	{
		sum += counter;
		counter++;
	}
	cout << "1 to 10 =" << sum << endl;
}

void TestIntegrator::Test3()
{
	int input = 0;
	int sum = 0;
	while ( cin >> input )
	{
		sum += input;
	}
	cout << "sum = " << sum << endl;
}
