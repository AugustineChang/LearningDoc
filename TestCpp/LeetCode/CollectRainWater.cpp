/*
给定 n 个非负整数表示每个宽度为 1 的柱子的高度图，计算按此排列的柱子，下雨之后能接多少雨水。
       #
   #~~~##~#
 #~##~######   # 表示柱子   ~ 表示水
上面是由数组 [0,1,0,2,1,0,1,3,2,1,2,1] 表示的高度图，在这种情况下，可以接 6 个单位的雨水（蓝色部分表示雨水）。 感谢 Marcos 贡献此图。

示例:
输入: [0,1,0,2,1,0,1,3,2,1,2,1]
输出: 6

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/trapping-rain-water
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <vector>
#include <iostream>
using namespace std;

struct Sink
{
	Sink( int i, int d )
		: index( i ), diff( d )
	{}

	int index;
	int diff;
};

class CollectRainWater 
{
public:
	int trap( vector<int>& height ) 
	{
		int stackTop = -1;
		vector<Sink> sinkStack;
		sinkStack.reserve( 5 );

		int totalWater = 0;
		int numOfList = height.size();
		for( int i = 1; i < numOfList; ++i )
		{
			int last = height[i - 1];
			int cur = height[i];

			if( last > cur )
			{
				sinkStack.push_back( Sink( i - 1, last - cur ) );
				++stackTop;
			}
			else if( last < cur )
			{
				int upDiff = cur - last;
				while( upDiff > 0 && stackTop >= 0 )
				{
					int downIndex = sinkStack[stackTop].index;
					int downDiff = sinkStack[stackTop].diff;

					if( downDiff > upDiff )
					{
						totalWater += upDiff * ( i - downIndex - 1 );
						
						sinkStack[stackTop].diff -= upDiff;
						upDiff = 0;
					}
					else
					{
						totalWater += downDiff * ( i - downIndex - 1 );
						upDiff -= downDiff;

						sinkStack.pop_back();
						--stackTop;
					}
				}
			}
		}
		return totalWater;
	}
};

//int main()
//{
//	CollectRainWater test;
//
//	//0,1,0,2,1,0,1,3,2,1,2,1
//	//4,0,1,0,1,2,3,3
//	vector<int> input = { 4,2,0,3,2,5 };
//	cout << test.trap( input ) << endl;
//
//	system( "pause" );
//	return 0;
//}