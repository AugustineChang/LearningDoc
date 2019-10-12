/*
将一个给定字符串根据给定的行数，以从上往下、从左到右进行 Z 字形排列。

比如输入字符串为 "LEETCODEISHIRING" 行数为 3 时，排列如下：

L   C   I   R
E T O E S I I G
E   D   H   N
之后，你的输出需要从左往右逐行读取，产生出一个新的字符串，比如："LCIRETOESIIGEDHN"。

请你实现这个将字符串进行指定行数变换的函数：

string convert(string s, int numRows);

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/zigzag-conversion
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/
#include <string>
#include <iostream>
using namespace std;

class ZConvert 
{
public:
	string convert( string s , int numRows ) 
	{
		if ( numRows <= 1 )
			return s;

		int repeatBase = ( numRows - 2 ) * 2 + 2;
		int totalLen = s.size();
		int numOfRepeat = totalLen / repeatBase;
		int remainder = totalLen - numOfRepeat * repeatBase;

		int* rowsLen = new int[numRows];
		char** allRows = new char*[numRows];
		for ( int i = 0; i < numRows; ++i )
		{
			int rowLen;
			if ( i == 0 || i == numRows - 1 )
				rowLen = numOfRepeat;
			else
				rowLen = numOfRepeat * 2;

			if ( i < remainder )
			{
				int other = 2 * ( numRows - 1 ) - i;
				if ( other != i && other < remainder )
					++rowLen;
				++rowLen;
			}

			allRows[i] = new char[rowLen];
			rowsLen[i] = 0;
		}

		for ( int i = 0; i < totalLen; ++i )
		{
			int rowIndex = ( i%repeatBase );
			if ( rowIndex >= numRows )
			{
				rowIndex = 2 * ( numRows - 1 ) - rowIndex;
			}
			
			allRows[rowIndex][rowsLen[rowIndex]] = s[i];
			++rowsLen[rowIndex];
		}

		/*
		//print for debug
		for ( int row = 0; row < numRows; ++row )
		{
			for ( int i = 0; i < rowsLen[row]; ++i )
			{
				cout << allRows[row][i];

				if ( row == 0 || row == numRows - 1 )
				{
					printSpace( numRows );
				}
				else
				{
					if ( i % 2 == 0 )
						printSpace( numRows - 1 - row );
					else
						printSpace( row );
				}

			}
			cout << endl;
		}*/

		int counter = 0;
		for ( int row = 0; row < numRows; ++row )
		{
			for ( int i = 0; i < rowsLen[row]; ++i )
			{
				s[counter] = allRows[row][i];
				++counter;
			}
		}

		return s;
	}

private:
	void printSpace( int count )
	{
		for ( int i = 0; i < count; ++i )
		{
			cout << ' ';
		}
	}
};

int main()
{
	string str( "ABCDE" );
	ZConvert test;
	cout << test.convert( str , 4 ) << endl;

	system( "pause" );
	return 0;
}