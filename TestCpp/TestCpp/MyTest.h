#pragma once
class MyTest
{
public:
	MyTest();
	~MyTest();

	int getNum() const;
	void setNum( int val );
	using myDelegate = int( MyTest::*)() const;

	int callFunc( MyTest *obj , myDelegate func );

	const MyTest &get() const {
		return *this;
	}

private:
	int number;
	static int a;
	friend MyTest addNum( const MyTest &A , const MyTest &B );
};

class Y;
class X
{
private:
	Y *pty;
};


class Y
{
private:
	X x;
};