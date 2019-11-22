/*
给出 n 代表生成括号的对数，请你写出一个函数，使其能够生成所有可能的并且有效的括号组合。

例如，给出 n = 3，生成结果为：

[
  "((()))",
  "(()())",
  "(())()",
  "()(())",
  "()()()"
]

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/generate-parentheses
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <vector>
#include <string>
#include <iostream>
using namespace std;

class GenerateBrackets 
{
public:
	vector<string> generateParenthesis( int n ) 
	{
		string emptyStr;
		vector<string> output;
		generateRecursively( 0, n, emptyStr, 2 * n, output );

		return output;
	}

private:

	void generateRecursively( int level, int remainingLeft, const string &curStr, int strLen, vector<string> &output )
	{
		if( curStr.size() >= strLen )
		{
			output.push_back( curStr );
			return;
		}

		if( remainingLeft > 0 )
		{
			string s1( curStr );
			s1.push_back( '(' );
			generateRecursively( level + 1, remainingLeft - 1, s1, strLen, output );
		}

		if( level > 0 )
		{
			string s2( curStr );
			s2.push_back( ')' );
			generateRecursively( level - 1, remainingLeft, s2, strLen, output );
		}

	}
};

//int main()
//{
//	GenerateBrackets test;
//
//	vector<string> output = test.generateParenthesis( 3 );
//
//	for( string &str : output )
//	{
//		cout << str << endl;
//	}
//
//	system( "pause" );
//	return 0;
//}