/*
给定一个只包括 '('，')'，'{'，'}'，'['，']' 的字符串，判断字符串是否有效。

有效字符串需满足：

左括号必须用相同类型的右括号闭合。
左括号必须以正确的顺序闭合。
注意空字符串可被认为是有效字符串。

示例 1:
输入: "()"
输出: true

示例 2:
输入: "()[]{}"
输出: true

示例 3:
输入: "(]"
输出: false

示例 4:
输入: "([)]"
输出: false

示例 5:
输入: "{[]}"
输出: true

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/valid-parentheses
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <string>
#include <iostream>
using namespace std;

class ValidBrackets
{
public:
	bool isValid( string s ) 
	{
		int strLen = s.size();
		if ( strLen <= 0 )
			return true;

		char *stack = new char[strLen];
		int top = -1;

		for ( int i = 0; i < strLen; ++i )
		{
			switch ( s[i] )
			{
			case '(':
			case '[':
			case '{':
				++top;
				stack[top] = s[i];
				break;

			case ')':
				if ( top < 0 || stack[top] != '(' )
					return false;
				--top;
				break;

			case ']':
				if ( top < 0 || stack[top] != '[' )
					return false;
				--top;
				break;

			case '}':
				if ( top < 0 || stack[top] != '{' )
					return false;
				--top;
				break;

			default:
				break;
			}
		}
		delete[] stack;

		return top < 0;
	}
};

int main()
{
	ValidBrackets test;

	cout << ( test.isValid( "([])" ) ? "true" : "false" ) << endl;

	system( "pause" );
	return 0;
}