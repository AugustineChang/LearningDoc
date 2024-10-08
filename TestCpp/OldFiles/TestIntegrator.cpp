#include "TestIntegrator.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
using namespace std;
using namespace std::placeholders;

TestIntegrator::TestIntegrator()
{
}


TestIntegrator::~TestIntegrator()
{
}

void TestIntegrator::Test1()
{
	int a = 1 , *pa = &a;
	cout << a << "," << pa << endl;
	pa++;
	cout << *pa << "," << pa << endl;

	const int c = 0 , &d = a;

	/*cout << "a=?,b=?" << endl;
	int a = 0 , b = 0;
	cin >> a >> b;

	cout << "a+b=" << ( a + b ) << endl;*/

	vector<int> vec = { 1,2,3,4,5 };
	auto iter = vec.begin();
	cout << *++iter << endl;
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

void TestIntegrator::Test4()
{
	int input;
	if ( !( cin >> input ) ) return;

	int lastInput = input;
	int counter = 1;
	while ( cin >> input )
	{
		if ( input == lastInput )
		{
			counter++;
		}
		else
		{
			cout << lastInput << ":" << counter << endl;
			counter = 1;
			lastInput = input;
		}
	}
	cout << lastInput << ":" << counter << endl;
}

void TestIntegrator::Test5()
{
	SalesData data1 , data2;

	double price;
	cin >> data1.bookName >> data1.salesNum >> price;
	data1.salesRevenue = data1.salesNum * price;

	cin >> data2.bookName >> data2.salesNum >> price;
	data2.salesRevenue = data2.salesNum * price;

	if ( data1.bookName == data2.bookName )
	{
		cout << "相同的书，合并两条销售记录" << endl;
		data1.salesNum += data2.salesNum;
		data1.salesRevenue += data2.salesRevenue;
		cout << data1.bookName << " " << data1.salesNum 
			 << " " << data1.salesRevenue << endl;
	}
	else
	{
		cout << "不同的书" << endl;
	}
}


bool isLarger( string str , size_t size )
{
	return str.size() >= size;
}

void TestIntegrator::Test6( int size )
{
	/*vector<int> alist = { 1,2,3,4,5,6 };
	using iterType = vector<int>::iterator;
	iterType result = find<iterType , int>( alist.begin() , alist.end() , 4 );
	if ( result != alist.end() )
	{
		cout << *result << endl;
	}*/

	vector<string> strList = { "aaa","bb","cccc","hhhhhhh","ggggg","d","eeeeeeeee" };
	/*auto ret = partition( strList.begin() , strList.end() , 
		[size]( const string &str ) -> bool { return str.size() > size; } );*/
	auto ret = partition( strList.begin() , strList.end() , bind( isLarger , _1 , size ) );
	for ( auto i = strList.begin(); i != ret; ++i )
	{
		cout << *i << ' ';
	}
	cout << endl;
}

