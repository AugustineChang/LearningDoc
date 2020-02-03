/*
给定两个以字符串形式表示的非负整数 num1 和 num2，返回 num1 和 num2 的乘积，它们的乘积也表示为字符串形式。

示例 1:
输入: num1 = "2", num2 = "3"
输出: "6"

示例 2:
输入: num1 = "123", num2 = "456"
输出: "56088"

说明：
num1 和 num2 的长度小于110。
num1 和 num2 只包含数字 0-9。
num1 和 num2 均不以零开头，除非是数字 0 本身。
不能使用任何标准库的大数类型（比如 BigInteger）或直接将输入转换为整数来处理。

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/multiply-strings
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <string>
#include <iostream>
using namespace std;

class MultiplyTwoNumbers 
{
public:
	string multiply( string num1, string num2 ) 
	{
		if( num1[0] == '0' || num2[0] == '0' )
			return string( "0" );

		string result;
		string temp;

		int numOfStr2 = num2.size();
		for( int i = 0; i < numOfStr2; ++i )
		{
			charMulStr( num1, num2[numOfStr2 - 1 - i], temp );

			int numOfTemp = temp.size();
			int len = numOfTemp + i - result.size();
			if( len > 0 )
				result.insert( 0, len, '0' );
			int numOfResults = result.size();

			bool hasCarry = false;
			char carry;
			int counter = 0;
			while( hasCarry || counter < numOfTemp )
			{
				int tarId = numOfResults - 1 - i - counter;
				int tempId = numOfTemp - 1 - counter;

				if( tarId < 0 )
				{
					tarId = 0;
					result.insert( 0, 1, '0' );
				}

				bool hasCarry1 = false, hasCarry2 = false;
				char carry1, carry2;

				if( hasCarry )
					hasCarry1 = charAddChar( result[tarId], carry, result[tarId], carry1 );

				if ( tempId >= 0 )
					hasCarry2 = charAddChar( result[tarId], temp[tempId], result[tarId], carry2 );

				char dummy;
				if( hasCarry1 && hasCarry2 )
					charAddChar( carry1, carry2, carry, dummy );
				else if( hasCarry1 )
					carry = carry1;
				else if( hasCarry2 )
					carry = carry2;
				hasCarry = hasCarry1 || hasCarry2;

				++counter;
			}	
		}

		return result;
	}

private:

	void charMulStr( const string &inStr, char inChar, string &outStr )
	{
		outStr.clear();
		bool hasCarry = false;
		char carry;

		int numOfStr = inStr.size();
		for( int i = numOfStr - 1; i >= 0; --i )
		{
			outStr.insert( 0, 1, ' ' );

			char mulOut, addOut;
			char mulCarry, addCarry;
			bool hasCarry1 = charMulChar( inStr[i], inChar, mulOut, mulCarry );

			bool hasCarry2 = false;
			if( hasCarry )
			{
				hasCarry2 = charAddChar( carry, mulOut, addOut, addCarry );
				outStr[0] = addOut;
			}
			else
				outStr[0] = mulOut;


			char dummy;
			if( hasCarry1 && hasCarry2 )
				charAddChar( mulCarry, addCarry, carry, dummy );
			else if( hasCarry1 )
				carry = mulCarry;
			else if( hasCarry2 )
				carry = addCarry;
			hasCarry = hasCarry1 || hasCarry2;
		}

		if( hasCarry )
			outStr.insert( 0, 1, carry );
	}

	bool charMulChar( char in1 , char in2, char &out, char &carry )
	{
		int num1 = static_cast<int>( in1 ) - 48;
		int num2 = static_cast<int>( in2 ) - 48;

		int ret = num1 * num2;
		out = static_cast<char>( ret % 10 + 48 );
		carry = static_cast<char>( ret / 10 + 48 );
		return ret >= 10;
	}

	bool charAddChar( char in1, char in2, char &out, char &carry )
	{
		int num1 = static_cast<int>( in1 ) - 48;
		int num2 = static_cast<int>( in2 ) - 48;

		int ret = num1 + num2;
		out = static_cast<char>( ret % 10 + 48 );
		carry = static_cast<char>( ret / 10 + 48 );
		return ret >= 10;
	}
};

int main()
{
	string a( "123456789" );
	string b( "987654321" );
	string output;

	MultiplyTwoNumbers test;
	output = test.multiply( a, b );

	cout << output << endl;

	system( "pause" );
	return 0;
}