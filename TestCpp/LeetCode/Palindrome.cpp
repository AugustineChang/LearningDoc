/*
给定一个字符串 s，找到 s 中最长的回文子串。你可以假设 s 的最大长度为 1000。

示例 1：
输入: "babad"
输出: "bab"
注意: "aba" 也是一个有效答案。

示例 2：
输入: "cbbd"
输出: "bb"

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/longest-palindromic-substring
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <iostream>
#include <vector>
#include <string>
using namespace std;

class Palindrome 
{
	struct PalindromeData
	{
		PalindromeData( int center , bool even )
			: centerIndex( center ) , length( 1 ) , isEven( even )
		{}

		int centerIndex;
		int length;
		bool isEven;
	};

public:
	string longestPalindrome( string s ) 
	{
		vector<PalindromeData> DataList;
		
		int strLen = s.size();
		int len2Index = -1;
		int len3Index = -1;
		for ( int i = 0; i < strLen; ++i )
		{
			bool isInLeft = i - 1 >= 0;
			bool isInRight = i + 1 < strLen;

			if ( isInLeft && isInRight &&s[i - 1] == s[i + 1] )
			{
				len3Index = DataList.size();
				DataList.push_back( PalindromeData( i , false ) );
			}

			if ( isInRight && s[i] == s[i + 1] )
			{
				len2Index = DataList.size();
				DataList.push_back( PalindromeData( i , true ) );
			}
		}
		
		//优先初始化为长度三的 其次初始化为长度二的
		int targetIndex = len3Index >= 0 ? len3Index : len2Index;
		int curLength = 1;
		int numOfCandidates = DataList.size();
		while ( true )
		{
			int numOfGrowth = 0;
			int evenIndex = -1;
			int oddIndex = -1;
			for ( int i = 0; i < numOfCandidates; ++i )
			{
				PalindromeData &CurData = DataList[i];
				if ( CurData.length < curLength )
					continue;

				int start = CurData.centerIndex - CurData.length;
				int end = CurData.centerIndex + CurData.length;
				if ( CurData.isEven )
					++start;

				int nextStart = start - 1;
				int nextEnd = end + 1;
				if ( nextStart >= 0 && nextEnd < strLen )
				{
					if ( s[nextStart] == s[nextEnd] )
					{
						++numOfGrowth;
						++CurData.length;
						if ( CurData.isEven )
							evenIndex = i;
						else
							oddIndex = i;
					}
				}
			}
			++curLength;

			if ( numOfGrowth <= 0 )
				break;
			else
				targetIndex = oddIndex >= 0 ? oddIndex : evenIndex;
		}

		int start , end;
		{
			//回文子串 长度仅为1
			if ( numOfCandidates <= 0 )
			{
				start = 0;
				end = 0;
			}
			else
			{
				PalindromeData &CurData = DataList[targetIndex];
				start = CurData.centerIndex - CurData.length;
				end = CurData.centerIndex + CurData.length;
				if ( CurData.isEven )
					++start;
			}
		}

		return s.substr( start , end - start + 1 );
	}
};

int main()
{
	string str( "bb" );
	Palindrome test;
	cout << test.longestPalindrome( str ) << endl;

	system( "pause" );
	return 0;
}