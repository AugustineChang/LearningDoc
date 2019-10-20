/*
给你一个字符串 s 和一个字符规律 p，请你来实现一个支持 '.' 和 '*' 的正则表达式匹配。

'.' 匹配任意单个字符
'*' 匹配零个或多个前面的那一个元素
所谓匹配，是要涵盖 整个 字符串 s的，而不是部分字符串。

说明:

s 可能为空，且只包含从 a-z 的小写字母。
p 可能为空，且只包含从 a-z 的小写字母，以及字符 . 和 *。

示例 1:
输入:
s = "aa"
p = "a"
输出: false
解释: "a" 无法匹配 "aa" 整个字符串。

示例 2:
输入:
s = "aa"
p = "a*"
输出: true
解释: 因为 '*' 代表可以匹配零个或多个前面的那一个元素, 在这里前面的元素就是 'a'。因此，字符串 "aa" 可被视为 'a' 重复了一次。

示例 3:
输入:
s = "ab"
p = ".*"
输出: true
解释: ".*" 表示可匹配零个或多个（'*'）任意字符（'.'）。

示例 4:
输入:
s = "aab"
p = "c*a*b"
输出: true
解释: 因为 '*' 表示零个或多个，这里 'c' 为 0 个, 'a' 被重复一次。因此可以匹配字符串 "aab"。

示例 5:
输入:
s = "mississippi"
p = "mis*is*p*."
输出: false

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/regular-expression-matching
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <string>
#include <vector>
#include <iostream>
using namespace std;

class SimpleRegex
{
	struct MatchData
	{
		char matchChar;
		bool isLoopMatch;
		vector<int> nextNodes;
	};

public:

	bool isMatch( string s , string p ) 
	{
		int strLen = s.size();
		int matchLen = p.size();

		if ( strLen <= 0 )
		{
			if ( matchLen <= 0 )
				return true;

			if ( matchLen % 2 == 0 )
			{
				bool isAllLoop = true;
				for ( int i = 1; i < matchLen; i += 2 )
				{
					if ( p[i] != '*' )
					{
						isAllLoop = false;
						break;
					}
				}
				return isAllLoop;
			}
			
			return false;
		}
		else if ( matchLen <= 0 )
		{
			return false;
		}

		// 分段 标准：一个字母/一个点/字母星号
		vector<MatchData> matchList;
		matchList.reserve( matchLen );
		for ( int i = 0; i < matchLen; ++i )
		{
			if ( p[i] == '*' && p[i + 1] == '*' )
				return false;

			MatchData newData;
			if ( i + 1 < matchLen && p[i + 1] == '*' )
			{
				newData.isLoopMatch = true;
				newData.matchChar = p[i];
				
				++i;
			}
			else
			{
				newData.isLoopMatch = false;
				newData.matchChar = p[i];
			}
			matchList.push_back( newData );
		}

		// 设定Next列表
		matchLen = matchList.size();
		for ( int i = 0; i < matchLen; ++i )
		{
			if ( matchList[i].isLoopMatch )//loop
			{
				matchList[i].nextNodes.push_back( i );
			}
			int nextI = i + 1;
			while ( nextI < matchLen )
			{
				matchList[i].nextNodes.push_back( nextI );
				if ( !matchList[nextI].isLoopMatch )
				{
					break;
				}
				++nextI;
			}
		}
		
		// 匹配
		if ( matchList[0].isLoopMatch )
		{
			int children = matchList[0].nextNodes.size();
			for ( int i = 0; i < children; ++i )
			{
				bool ret = Match( matchList , s , 0 , matchList[0].nextNodes[i] );
				if ( ret )
					return true;
			}
			return false;
		}
		else
		{
			return Match( matchList , s , 0 , 0 );
		}
	}

private:

	bool Match( const vector<MatchData> &matchList , const string &s , int strPtr , int matchPtr )
	{
		const MatchData &data = matchList[matchPtr];
		int strLen = s.size();
		int childStart = 0;
		if ( data.isLoopMatch )
		{
			if ( strPtr >= strLen || ( data.matchChar != '.' && data.matchChar != s[strPtr] ) )
			{
				childStart = 1;
			}
			else
			{
				++strPtr;
			}
		}
		else
		{
			if ( strPtr >= strLen )
				return false;
			if ( data.matchChar != '.' && data.matchChar != s[strPtr] )
				return false;

			++strPtr;
		}
		
		bool isLastMatch = matchPtr == matchList.size() - 1;
		if ( isLastMatch && strPtr == s.size() )
			return true;

		int children = data.nextNodes.size();
		for ( int i = childStart; i < children; ++i )
		{
			bool ret = Match( matchList , s , strPtr , data.nextNodes[i] );
			if ( ret )
				return true;
		}
		return false;
	}
	
};

//int main()
//{
//	SimpleRegex test;
//
//	string s1( "ab" );
//	string s2( ".*.." );
//
//	cout << ( test.isMatch( s1 , s2 ) ? "true" : "false" ) << endl;
//
//	system( "pause" );
//	return 0;
//}