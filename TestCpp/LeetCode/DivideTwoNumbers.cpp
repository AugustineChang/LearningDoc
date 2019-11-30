/*
给定两个整数，被除数 dividend 和除数 divisor。将两数相除，要求不使用乘法、除法和 mod 运算符。
返回被除数 dividend 除以除数 divisor 得到的商。

示例 1:
输入: dividend = 10, divisor = 3
输出: 3

示例 2:
输入: dividend = 7, divisor = -3
输出: -2

说明:
被除数和除数均为 32 位有符号整数。
除数不为 0。
假设我们的环境只能存储 32 位有符号整数，其数值范围是 [−231,  231 − 1]。本题中，如果除法结果溢出，则返回 231 − 1。

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/divide-two-integers
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <iostream>
using namespace std;

class DivideTwoNumbers 
{
public:
	int divide( int dividend, int divisor ) 
	{
		if( dividend == INT_MIN && divisor == -1 )
			return INT_MAX;
		if( dividend == INT_MIN && divisor == 1 )
			return INT_MIN;
		if( divisor == INT_MIN )
			return dividend == INT_MIN ? 1 : 0;

		int isPositive = true;
		if( divisor < 0 )
		{
			isPositive = !isPositive;
			divisor = -divisor;
		}

		int remainder = 0;
		if( dividend < 0 )
		{
			isPositive = !isPositive;
			if( dividend == INT_MIN )
			{
				dividend = INT_MAX;
				remainder = 1;
			}
			else
			{
				dividend = -dividend;
			}
		}
		
		int quotient = 0;
		int halfMax = 1073741824;
		while( dividend >= divisor )
		{
			int lastDivisor = divisor;
			int lastQuotient = 1;
			int curDivisor = divisor;
			int curQuotient = 1;
			while( curDivisor <= dividend && curDivisor < halfMax )
			{
				lastDivisor = curDivisor;
				lastQuotient = curQuotient;

				curDivisor = curDivisor << 1;
				curQuotient = curQuotient << 1;
			}
			dividend -= lastDivisor;
			quotient += lastQuotient;

			if( remainder > 0 )
			{
				dividend += remainder;
				remainder = 0;
			}
		}

		return isPositive ? quotient : -quotient;
	}
};

int main()
{
	DivideTwoNumbers test;

	cout << test.divide( 1100540749, -1090366779 ) << endl;

	system( "pause" );
	return 0;
}