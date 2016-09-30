// TestCpp.cpp : Defines the entry point for the console application.
//
#include <stdlib.h>
#include "TestIntegrator.h"

int main()
{
	TestIntegrator* ti = new TestIntegrator();
	ti->Test3();

	system( "Pause" );
    return 0;
}

