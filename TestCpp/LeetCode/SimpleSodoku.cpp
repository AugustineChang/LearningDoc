/*
判断一个 9x9 的数独是否有效。只需要根据以下规则，验证已经填入的数字是否有效即可。

数字 1-9 在每一行只能出现一次。
数字 1-9 在每一列只能出现一次。
数字 1-9 在每一个以粗实线分隔的 3x3 宫内只能出现一次。

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/valid-sudoku
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <vector>
#include <unordered_set>
#include <iostream>
using namespace std;

class SimpleSodoku 
{
public:
	bool isValidSudoku( vector<vector<char>>& board ) 
	{
		unordered_set<char> rowSets[9];
		unordered_set<char> colSets[9];
		unordered_set<char> subSets[9];

		for( int y = 0; y < 9; ++y )
		{
			for( int x = 0; x < 9; ++x )
			{
				char curChar = board[y][x];
				if( curChar == '.' )
					continue;

				unordered_set<char>::const_iterator rowIter = rowSets[y].find( curChar );
				if( rowIter != rowSets[y].end() )
					return false;
				rowSets[y].emplace( curChar );

				unordered_set<char>::const_iterator colIter = colSets[x].find( curChar );
				if( colIter != colSets[x].end() )
					return false;
				colSets[x].emplace( curChar );

				int subId = ( y / 3 ) * 3 + ( x / 3 );
				unordered_set<char>::const_iterator subIter = subSets[subId].find( curChar );
				if( subIter != subSets[subId].end() )
					return false;
				subSets[subId].emplace( curChar );
			}
		}

		return true;
	}
};

int main()
{
	vector<vector<char>> input = 
	{
		{'5','3','.','.','7','.','.','.','.'},
		{'6','.','.','1','9','5','.','6','.'},
		{'.','9','8','.','.','.','.','6','.'},
		{'8','.','.','.','6','.','.','.','3'},
		{'4','.','.','8','.','3','.','.','1'},
		{'7','.','.','.','2','.','.','.','6'},
		{'.','6','.','.','.','.','2','8','.'},
		{'.','.','.','4','1','9','.','.','5'},
		{'.','.','.','.','8','.','.','7','9'}
	};

	SimpleSodoku test;
	cout << test.isValidSudoku( input ) << endl;

	system( "pause" );
	return 0;
}