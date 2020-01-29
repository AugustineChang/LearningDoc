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
#include <map>
#include <windows.h>
#include <iostream>
#include <assert.h>
using namespace std;

struct IntPoint
{
	IntPoint() 
		: x( 0 ), y( 0 )
	{}
	IntPoint( int _x, int _y ) 
		: x( _x ), y( _y )
	{}

	bool operator==( const IntPoint &other ) const 
	{
		return x == other.x && y == other.y;
	}

	bool operator!=( const IntPoint &other ) const
	{
		return !(*this == other);
	}

	bool operator<( const IntPoint &other ) const 
	{
		if( y == other.y )
		{
			return x < other.x;
		}
		else
		{
			return y < other.y;
		}
	}

	int x;
	int y;
};

struct EmptyCell
{
	EmptyCell()
		: resultChar( '.' ), guessChar( '.' )
	{
		range.reserve( 9 );
		for( int i = 0; i < 9; ++i )
		{
			range.emplace( static_cast<char>( 49 + i ) );
		}
	}

	bool remove( char val )
	{
		return range.erase( val ) > 0;
	}

	void add( char val )
	{
		range.emplace( val );
	}

	bool isGuessed()
	{
		return guessChar != '.';
	}

	bool isEmpty() 
	{
		return range.empty();
	}

	int getProbRange()
	{
		if( resultChar != '.' || isGuessed() )
			return 0;
		else
			return range.size();
	}

	char getValidChar()
	{
		assert( range.size() > 0 );
		return *range.begin();
	}
	
	char resultChar;
	unordered_set<char> range;

	//guess
	char guessChar;
};

struct GuessData
{
	GuessData()
		: GuessPos( -1, -1 ), GuessChar( '.' )
	{}

	IntPoint GuessPos;
	char GuessChar;

	map<IntPoint, vector<char>> ExcludeByGuess;
};

class SimpleSodoku 
{
public:
	bool isValidSudoku( vector<vector<char>>& board );

	void solveSudoku( vector<vector<char>>& board ) 
	{
		baseBoard = board;

		loadSudoku( board, needToFill );
		numOfFilled = 0;
		if( !fillSudoku() )
			return;

		curGuessLevel = 0;
		GuessData newData;
		GuessStack.reserve( 5 );
		GuessStack.push_back( newData );

		if( !doGuessRecursively() )
			return;
		
		//vector<IntPoint> dummy;
		//printSodoku( board, dummy );

		for( auto iter = selectionRange.begin(); iter != selectionRange.end(); ++iter )
		{
			const IntPoint &key = iter->first;
			EmptyCell &cell = iter->second;

			board[key.y][key.x] = cell.isGuessed() ? cell.guessChar : cell.resultChar;
		}
		//isValidSudoku( board );
	}

private:

	bool doGuessRecursively();

	void loadSudoku( vector<vector<char>>& board, int &cellToFill );
	bool fillSudoku( bool doPrint = false );
	bool guessSudoku( const IntPoint &key, char guessChar , bool doPrint = false );
	void fallbackGuess();
	bool updateSelectRange( vector<IntPoint> &changedKeys );
	void printSodoku( vector<vector<char>>& board, vector<IntPoint> &changedCells );

private:
	
	vector<vector<char>> baseBoard;

	map<IntPoint, EmptyCell> selectionRange;
	int needToFill;
	int numOfFilled;

	//guess stack
	int curGuessLevel;
	vector<GuessData> GuessStack;
};

bool SimpleSodoku::isValidSudoku( vector<vector<char>>& board )
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
			{
				cout << "Error:(" << x << "," << y << ")" << endl;
				return false;
			}
			rowSets[y].emplace( curChar );

			unordered_set<char>::const_iterator colIter = colSets[x].find( curChar );
			if( colIter != colSets[x].end() )
			{
				cout << "Error:(" << x << "," << y << ")" << endl;
				return false;
			}
			colSets[x].emplace( curChar );

			int subId = ( y / 3 ) * 3 + ( x / 3 );
			unordered_set<char>::const_iterator subIter = subSets[subId].find( curChar );
			if( subIter != subSets[subId].end() )
			{
				cout << "Error:(" << x << "," << y << ")" << endl;
				return false;
			}
			subSets[subId].emplace( curChar );
		}
	}

	cout << "Correct!" << endl;
	return true;
}

bool SimpleSodoku::doGuessRecursively()
{
	if( numOfFilled >= needToFill )
		return true;

	vector<IntPoint> candiKey;
	vector<char> candiChar;
	
	for( auto iter = selectionRange.begin(); iter != selectionRange.end(); ++iter )
	{
		const IntPoint &key = iter->first;
		EmptyCell &cell = iter->second;

		int numOfValid = cell.getProbRange();
		if( numOfValid <= 0 )
			continue;

		vector<char> removeChars;
		for( auto iter2 = cell.range.begin(); iter2 != cell.range.end(); ++iter2 )
		{
			char tarChar = *iter2;
			bool ret = guessSudoku( key, tarChar , false );
			ret = ret && fillSudoku( false );
			
			if( ret )//成功的猜测
			{
				candiKey.push_back( key );
				candiChar.push_back( tarChar );
			}
			else
			{
				removeChars.push_back( tarChar );
			}
			fallbackGuess();
		}
		
		map<IntPoint, vector<char>> &excludeMap = GuessStack[curGuessLevel].ExcludeByGuess;
		for( char val : removeChars )
		{
			if( cell.remove( val ) )
				excludeMap[key].push_back( val );
		}
		if( cell.range.size() == 0 )
		{
			return false;
		}
	}

	fillSudoku();

	int numOfCandidates = candiKey.size();
	if( numOfCandidates <= 0 )
		return false;

	for( int i = 0; i < numOfCandidates; ++i )
	{
		EmptyCell &cell = selectionRange[candiKey[i]];
		if ( cell.resultChar != '.' )
			continue;

		bool ret = guessSudoku( candiKey[i], candiChar[i] );
		ret = ret && fillSudoku();
		if( !ret )
		{
			fallbackGuess();
			continue;
		}

		//cout << "level" << curGuessLevel << '(' << i << '/' << numOfCandidates << ')' << ':';
		//cout << numOfFilled << '/' << needToFill << endl;

		if( doGuessRecursively() )
			return true;
		else
		{
			fallbackGuess();
		}
	}

	return false;
}

void SimpleSodoku::loadSudoku( vector<vector<char>>& board, int &cellToFill )
{
	cellToFill = 0;
	for( int y = 0; y < 9; ++y )
	{
		for( int x = 0; x < 9; ++x )
		{
			if( board[y][x] != '.' )
				continue;

			IntPoint key( x, y );
			EmptyCell &cell = selectionRange[key];

			int subStartX = ( x / 3 ) * 3;
			int subStartY = ( y / 3 ) * 3;
			for( int i = 0; i < 9; ++i )
			{
				char rowChar = board[y][i];
				if( rowChar != '.' )
				{
					cell.remove( rowChar );
				}

				char colChar = board[i][x];
				if( colChar != '.' )
				{
					cell.remove( colChar );
				}

				int subX = subStartX + ( i % 3 );
				int subY = subStartY + ( i / 3 );
				char subChar = board[subY][subX];
				if( subChar != '.' )
				{
					cell.remove( subChar );
				}
			}

			++cellToFill;
		}
	}
}

bool SimpleSodoku::fillSudoku( bool doPrint )
{
	while( true )
	{
		//遍历所有未填写的格子
		vector<IntPoint> changedKeys;
		for( auto iter = selectionRange.begin(); iter != selectionRange.end(); ++iter )
		{
			const IntPoint &key = iter->first;
			EmptyCell &cell = iter->second;
			int numOfValid = cell.getProbRange();
			if( numOfValid == 1 )//填写确定的格子
			{
				changedKeys.push_back( key );
				cell.resultChar = *( cell.range.begin() );
				++numOfFilled;
			}
		}

		int numOfChanged = changedKeys.size();
		if( numOfChanged <= 0 )
		{
			break;
		}
		
		//改写其他格
		bool ret = updateSelectRange( changedKeys );
		if ( doPrint ) 
			printSodoku( baseBoard, changedKeys );
		if( !ret )
			return false;//改写过程出错
	}

	return true;
}

bool SimpleSodoku::guessSudoku( const IntPoint &key , char guessChar , bool doPrint )
{
	//随便选一个（选第一个有效的）
	EmptyCell &cell = selectionRange[key];
	cell.guessChar = guessChar;
	++numOfFilled;
	
	++curGuessLevel;
	GuessData newData;
	newData.GuessChar = guessChar;
	newData.GuessPos = key;
	GuessStack.push_back( newData );

	//改写其他格
	vector<IntPoint> keyList;
	keyList.push_back( key );
	
	bool ret = updateSelectRange( keyList );
	if( doPrint )
		printSodoku( baseBoard, keyList );
	return ret;
}

void SimpleSodoku::fallbackGuess()
{
	//回退猜想
	IntPoint &guessPos = GuessStack[curGuessLevel].GuessPos;
	selectionRange[guessPos].guessChar = '.';
	--numOfFilled;

	//回退其他格
	map<IntPoint, vector<char>> &excludeMap = GuessStack[curGuessLevel].ExcludeByGuess;
	for( auto Iter1 = excludeMap.begin(); Iter1 != excludeMap.end(); ++Iter1 )
	{
		const IntPoint &key = Iter1->first;
		EmptyCell &cell = selectionRange[key];

		vector<char> &charlist = Iter1->second;
		for( auto Iter2 = charlist.begin(); Iter2 != charlist.end(); ++Iter2 )
		{
			cell.add( *Iter2 );
		}
		if( cell.resultChar != '.' )
		{
			--numOfFilled;
			cell.resultChar = '.';
		}
	}
	
	GuessStack.pop_back();
	--curGuessLevel;
}

bool SimpleSodoku::updateSelectRange( vector<IntPoint> &changedKeys )
{
	bool result = true;
	map<IntPoint, vector<char>> &excludeMap = GuessStack[curGuessLevel].ExcludeByGuess;

	//填写之后 其他格子可能性改变
	int numOfChanged = changedKeys.size();
	for( int i = 0; i < numOfChanged; ++i )
	{
		IntPoint &key = changedKeys[i];
		EmptyCell &cell = selectionRange[key];
		char curChar = cell.isGuessed() ? cell.guessChar : cell.resultChar;

		//改写 同行 同列 同九宫的
		int subStartX = ( key.x / 3 ) * 3;
		int subStartY = ( key.y / 3 ) * 3;
		for( int k = 0; k < 9; ++k )
		{
			IntPoint rowKey( k, key.y );
			auto rowIter = selectionRange.find( rowKey );
			if( k != key.x  && rowIter != selectionRange.end() )
			{
				EmptyCell &rowCell = rowIter->second;
				if( rowCell.guessChar == curChar || rowCell.resultChar == curChar )
				{
					//cout << "Error:(" << rowKey.x << "," << rowKey.y << ")" << endl;
					//result = false;//猜测是错误的
					return false;
				}
				else if ( rowCell.getProbRange() > 0 )
				{
					if( rowCell.remove( curChar ) )
						excludeMap[rowKey].push_back( curChar );
				}
			}

			IntPoint colKey( key.x, k );
			auto colIter = selectionRange.find( colKey );
			if( k != key.y && colIter != selectionRange.end() )
			{
				EmptyCell &colCell = colIter->second;
				if( colCell.guessChar == curChar || colCell.resultChar == curChar )
				{
					//cout << "Error:(" << colKey.x << "," << colKey.y << ")" << endl;
					//result = false;//猜测是错误的
					return false;
				}
				else if( colCell.getProbRange() > 0 )
				{
					if( colCell.remove( curChar ) )
						excludeMap[colKey].push_back( curChar );
				}
			}

			IntPoint subKey( subStartX + ( k % 3 ), subStartY + ( k / 3 ) );
			auto subIter = selectionRange.find( subKey );
			if( subKey != key && subIter != selectionRange.end() )
			{
				EmptyCell &subCell = subIter->second;
				if( subCell.guessChar == curChar || subCell.resultChar == curChar )
				{
					//cout << "Error:(" << subKey.x << "," << subKey.y << ")" << endl;
					//result = false;//猜测是错误的
					return false;
				}
				else if( subCell.getProbRange() > 0 )
				{
					if( subCell.remove( curChar ) )
						excludeMap[subKey].push_back( curChar );
				}
			}
		}
	}

	return true;
}

void SimpleSodoku::printSodoku( vector<vector<char>>& board , vector<IntPoint> &changedCells )
{
	cout << "======================" << endl << endl;

	for( int y = 0; y < 9; ++y )
	{
		for( int x = 0; x < 9; ++x )
		{
			IntPoint key( x, y );

			if( board[y][x] != '.' )
			{
				cout << board[y][x] << ' ';
			}
			else if( selectionRange.find( key ) != selectionRange.end() )
			{
				EmptyCell &cell = selectionRange[key];

				unsigned short color = 0;
				int num = changedCells.size();
				for( int i = 0; i < num; ++i )
				{
					if( key == changedCells.at( i ) )
					{
						color = cell.isGuessed() ? FOREGROUND_GREEN : FOREGROUND_RED;
						break;
					}
				}
				
				if( color > 0 )
				{
					SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), color );
				}

				if ( cell.resultChar != '.' )
					cout << cell.resultChar << ' ';
				else if ( cell.isGuessed() )
					cout << cell.guessChar << ' ';
				else
					cout << ". ";

				if( color > 0 )
				{
					SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE );
				}
			}

			if( x == 2 || x == 5 )
			{
				cout << '|';
			}
		}
		cout << endl;

		if( y == 2 || y == 5 )
		{
			cout << "-------------------" << endl;
		}
	}
	cout << endl;
}

int main()
{
	vector<vector<char>> input =
	{		
		{'.','1','.','4','.','.','.','5','.'},
		{'7','.','.','.','.','6','.','.','.'},
		{'.','.','.','.','.','.','.','.','.'},
		{'.','4','.','1','5','.','.','3','.'},
		{'.','.','.','.','.','.','.','.','2'},
		{'.','.','.','9','.','.','.','.','6'},
		{'.','.','.','.','.','.','9','1','.'},
		{'.','.','.','.','.','.','.','.','.'},
		{'6','.','2','.','.','7','.','.','.'}
	};

	SimpleSodoku test;

	test.solveSudoku( input );
	//test.isValidSudoku( input );

	system( "pause" );
	return 0;
}