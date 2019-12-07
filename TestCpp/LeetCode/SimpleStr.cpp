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
#include <vector>
#include <unordered_map>
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
	
	vector<int> findSubstring( string s, vector<string>& words ) 
	{
		vector<int> output;
		int numOfWords = words.size();
		int wordSize = numOfWords > 0 ? words[0].size() : 0;
		int stringSize = s.size();
		int subStrSize = numOfWords * wordSize;

		if ( wordSize <= 0 )
			return output;
		if( stringSize < subStrSize )
			return output;

		unordered_map<string, int> standardMap;
		unordered_map<string, int> curMap;

		//init standard map
		standardMap.reserve( numOfWords );
		for( int i = 0; i < numOfWords; ++i )
		{
			if( standardMap.find( words[i] ) == standardMap.end() )
			{
				standardMap.emplace( words[i], 1 );
			}
			else
			{
				++standardMap[words[i]];
			}
		}

		int startIndex = 0;
		while( startIndex + subStrSize <= stringSize )
		{
			curMap.clear();

			bool isFound = true;
			for( int i = 0; i < numOfWords; ++i )
			{
				string curSubStr = s.substr( startIndex + i * wordSize, wordSize );
				auto standardIter = standardMap.find( curSubStr );
				if( standardIter == standardMap.end() )// unknown word
				{
					++startIndex;
					isFound = false;
					break;
				}
				else
				{
					unordered_map<string, int>::iterator curIter = curMap.find( curSubStr );
					if( curIter == curMap.end() )// first
					{
						curMap.emplace( curSubStr, 1 );
					}
					else
					{
						++( curIter->second );

						if( curIter->second > standardIter->second )// too many words
						{
							++startIndex;
							isFound = false;
							break;
						}
					}
				}
			}

			if( isFound )
			{
				output.push_back( startIndex );
				++startIndex;
			}
		}

		return output;
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
//	//cout << test.strStr( "a", "" ) << endl;
//	vector<string> input = 
//	{
//		"ab","ba","ba"
//	};
//
//	string str( "ababaab" );
//	vector<int> output = test.findSubstring( str, input );
//	for( int val : output )
//	{
//		cout << val << " ";
//	}
//	cout << endl;
//
//	system( "pause" );
//	return 0;
//}