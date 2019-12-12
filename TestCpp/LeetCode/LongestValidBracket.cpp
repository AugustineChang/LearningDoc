/*
给定一个只包含 '(' 和 ')' 的字符串，找出最长的包含有效括号的子串的长度。

示例 1:
输入: "(()"
输出: 2
解释: 最长有效括号子串为 "()"

示例 2:
输入: ")()())"
输出: 4
解释: 最长有效括号子串为 "()()"

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/longest-valid-parentheses
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <string>
#include <iostream>
using namespace std;

#define Max(a,b) (a) > (b) ? (a) : (b)

class LongestValidBracket
{
public:
	int longestValidParentheses( string s ) 
	{
		int strSize = s.size();
		
		int level = 0;
		int *stack = new int[strSize];
		bool *validList = new bool[strSize];
		for( int i = 0; i < strSize; ++i )
			validList[i] = false;

		for( int i = 0; i < strSize; ++i )
		{
			char curChar = s[i];
			if( curChar == '(' )
			{
				stack[level] = i;
				++level;
			}
			else if( curChar == ')' )
			{
				if( level > 0 )
				{
					--level;
					validList[stack[level]] = true;
					validList[i] = true;
				}
			}
		}

		int maxValidLen = 0;
		int validLen = 0;
		for( int i = 0; i < strSize; ++i )
		{
			//cout << ( validList[i] ? "true " : "false " );
			if( validList[i] )
			{
				++validLen;
			}
			else if ( validLen > 0 )
			{
				maxValidLen = Max( validLen, maxValidLen );
				validLen = 0;
			}
		}
		//cout << endl;

		delete[] stack;
		delete[] validList;
		
		maxValidLen = Max( validLen, maxValidLen );
		return maxValidLen;
	}
};

//int main()
//{
//	LongestValidBracket test;
//
//	cout << test.longestValidParentheses( "(()" ) << endl;
//
//	system( "pause" );
//	return 0;
//}