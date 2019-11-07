/*
给定一个仅包含数字 2-9 的字符串，返回所有它能表示的字母组合。

给出数字到字母的映射如下（与电话按键相同）。注意 1 不对应任何字母。

示例:

输入："23"
输出：["ad", "ae", "af", "bd", "be", "bf", "cd", "ce", "cf"].
说明:
尽管上面的答案是按字典序排列的，但是你可以任意选择答案输出的顺序。

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/letter-combinations-of-a-phone-number
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <vector>
#include <string>
#include <cmath>
#include <iostream>
using namespace std;

class MobileInput 
{
public:
	vector<string> letterCombinations( string digits ) 
	{
		vector<string> output;
		int strLen = digits.size();
		if ( strLen <= 0 )
			return output;

		int tableLen[8] = { 3, 3, 3, 3, 3, 4, 3, 4 };
		char charTable[8][4] =
		{
			{ 'a','b','c',' ' },
			{ 'd','e','f',' ' },
			{ 'g','h','i',' ' },
			{ 'j','k','l',' ' },
			{ 'm','n','o',' ' },
			{ 'p','q','r','s' },
			{ 't','u','v',' ' },
			{ 'w','x','y','z' }
		};
		
		vector<int> digitNums( strLen );

		int totalNum = 1;
		for ( int i = 0; i < strLen; ++i )
		{
			digitNums[i] = char2int( digits[i] );
			totalNum *= tableLen[digitNums[i]];
		}
		output.reserve( totalNum );
		
		bool isEnd = false;
		vector<int> stateTable( strLen , 0 );
		while ( !isEnd )
		{
			string oneRet;
			oneRet.reserve( strLen );
			for ( int i = 0; i < strLen; ++i )
			{
				oneRet.push_back( charTable[digitNums[i]][stateTable[i]] );
			}
			output.push_back( oneRet );

			++stateTable[strLen - 1];
			for ( int i = strLen - 1; i >= 0; --i )
			{
				if ( stateTable[i] >= tableLen[digitNums[i]] )
				{
					isEnd = i == 0;
					if ( !isEnd )
					{
						stateTable[i] = 0;
						++stateTable[i - 1];
					}
				}
				else
					break;
			}
		}

		return output;
	}

private:

	int char2int( char c )
	{
		int charCode = static_cast<int>( c );
		return charCode - 50;
	}
};

//int main()
//{
//	MobileInput test;
//
//	vector<string> output = test.letterCombinations( "" );
//	for ( const string &oneOut : output )
//	{
//		cout << oneOut << ' ';
//	}
//	cout << endl;
//
//	system( "pause" );
//	return 0;
//}