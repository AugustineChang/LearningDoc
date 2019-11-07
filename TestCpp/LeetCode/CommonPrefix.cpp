/*
编写一个函数来查找字符串数组中的最长公共前缀。

如果不存在公共前缀，返回空字符串 ""。

示例 1:
输入: ["flower","flow","flight"]
输出: "fl"

示例 2:
输入: ["dog","racecar","car"]
输出: ""
解释: 输入不存在公共前缀。

说明:
所有输入只包含小写字母 a-z 。

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/longest-common-prefix
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <string>
#include <vector>
#include <iostream>
using namespace std;

class CommonPrefix 
{
public:
	string longestCommonPrefix( vector<string>& strs ) 
	{
		int numOfStrs = strs.size();
		if ( numOfStrs <= 0 )
			return string();
		else if ( numOfStrs == 1 )
			return strs[0];

		int minSize = strs[0].size();
		for ( int i = 1; i < numOfStrs; ++i )
		{
			int curSize = strs[i].size();
			if ( curSize < minSize )
			{
				minSize = curSize;
			}
		}

		int charPtr = 0;
		bool isEnd = false;
		while ( !isEnd && charPtr < minSize )
		{
			for ( int i = 1; i < numOfStrs; ++i )
			{
				if ( strs[i][charPtr] != strs[0][charPtr] )
				{
					isEnd = true;
					break;
				}
			}

			if ( !isEnd )
				++charPtr;
		}
		
		return strs[0].substr( 0 , charPtr );
	}
};

//int main()
//{
//	vector<string> input = { "","" };
//
//	CommonPrefix test;
//	cout << test.longestCommonPrefix( input ) << endl;
//
//	system( "pause" );
//	return 0;
//}