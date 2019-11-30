/*
实现 strStr() 函数。
给定一个 haystack 字符串和一个 needle 字符串，在 haystack 字符串中找出 needle 字符串出现的第一个位置 (从0开始)。如果不存在，则返回  -1。

示例 1:
输入: haystack = "hello", needle = "ll"
输出: 2

示例 2:
输入: haystack = "aaaaa", needle = "bba"
输出: -1

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/implement-strstr
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <string>
#include <iostream>
using namespace std;

class SimpleStr 
{
public:
	int strStr( string haystack, string needle ) 
	{
		int curIndex = 0;
		int baseLen = haystack.size();
		int patternLen = needle.size();
		while( curIndex + patternLen <= baseLen )
		{
			bool isMatch = match( haystack, needle, curIndex );
			if( isMatch )
			{
				return curIndex;
			}
			else
			{
				if ( curIndex + patternLen >= baseLen )
					break;

				int offset = getOffset( needle, haystack[curIndex + patternLen] );
				curIndex += offset;
			}
		}

		return -1;
	}

private:

	bool match( const string &base, const string &pattern, int baseIndex )
	{
		bool result = true;
		int len = pattern.size();
		for( int i = 0; i < len; ++i )
		{
			if( base[baseIndex + i] != pattern[i] )
			{
				result = false;
				break;
			}
		}

		return result;
	}

	int getOffset( const string &pattern, char target )
	{
		int len = pattern.size();
		for( int i = len - 1; i >= 0; --i )
		{
			if( pattern[i] == target )
			{
				return len - i;
			}
		}
		return len + 1;
	}
};

//int main()
//{
//	SimpleStr test;
//
//	cout << test.strStr( "a" , "" ) << endl;
//
//	system( "pause" );
//	return 0;
//}