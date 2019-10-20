/*
判断一个整数是否是回文数。回文数是指正序（从左向右）和倒序（从右向左）读都是一样的整数。

示例 1:
输入: 121
输出: true

示例 2:
输入: -121
输出: false
解释: 从左向右读, 为 -121 。 从右向左读, 为 121- 。因此它不是一个回文数。

示例 3:
输入: 10
输出: false
解释: 从右向左读, 为 01 。因此它不是一个回文数。

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/palindrome-number
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <iostream>

class PalindromeInt 
{
public:
	bool isPalindrome( int x ) 
	{
		if ( x < 0 )
			return false;

		int decBits[32];

		int curBit = 0;
		while ( x > 0 )
		{
			decBits[curBit] = x % 10;
			x /= 10;
			++curBit;
		}

		int mid = curBit / 2;
		bool isEven = curBit % 2 == 0;
		int left = isEven ? mid - 1 : mid;
		int right = mid;

		while ( left >= 0 && right < curBit )
		{
			if ( decBits[left] != decBits[right] )
			{
				return false;
			}

			--left;
			++right;
		}

		return true;
	}
};

//int main()
//{
//	PalindromeInt test;
//
//	std::cout << ( test.isPalindrome( 100 ) ? "true" : "false" ) << std::endl;
//
//	system( "pause" );
//	return 0;
//}