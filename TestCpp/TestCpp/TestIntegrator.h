#pragma once
class TestIntegrator
{
public:
	TestIntegrator();
	~TestIntegrator();
	
	void Test1();
	void Test2();
	void Test3();
	void Test4();
	void Test5();
};

#ifndef SALES_DATA
#define SALES_DATA

#include <string>
struct SalesData
{
	std::string bookName;
	int salesNum;
	double salesRevenue;
};

#endif
