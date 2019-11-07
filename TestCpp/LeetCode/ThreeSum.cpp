/*
给定一个包含 n 个整数的数组 nums，判断 nums 中是否存在三个元素 a，b，c ，使得 a + b + c = 0 ？找出所有满足条件且不重复的三元组。

注意：答案中不可以包含重复的三元组。

例如, 给定数组 nums = [-1, 0, 1, 2, -1, -4]，

满足要求的三元组集合为：
[
  [-1, 0, 1],
  [-1, -1, 2]
]

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/3sum
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <vector>
#include <iostream>
using namespace std;

class ThreeSum
{
public:
	vector<vector<int>> threeSum( vector<int>& nums ) 
	{
		vector<vector<int>> results;
		int numOfList = nums.size();
		if ( numOfList <= 0 )
			return results;

		QuickSort( nums , 0 , numOfList - 1 );

		for ( int i = 0; i < numOfList - 2; ++i )
		{
			if ( i > 0 && nums[i-1] == nums[i] )
				continue;

			int head = i + 1;
			int tail = numOfList - 1;
			while ( true )
			{
				int sum = nums[i] + nums[head] + nums[tail];
				if ( sum < 0 )
				{	
					int curVal = nums[head];
					while ( head < tail - 1 && nums[head] == curVal )
						++head;

					if ( nums[head] == curVal )
						break;
				}
				else if ( sum > 0 )
				{
					int curVal = nums[tail];
					while ( tail > head + 1 && nums[tail] == curVal )
						--tail;
					
					if ( nums[tail] == curVal )
						break;
				}
				else
				{
					vector<int> oneRet;
					oneRet.reserve( 3 );

					oneRet.push_back( nums[head] );
					oneRet.push_back( nums[i] );
					oneRet.push_back( nums[tail] );

					results.push_back( oneRet );

					int curHead = nums[head];
					while ( head < tail - 1 && nums[head] == curHead )
						++head;

					int curTail = nums[tail];
					while ( tail > head + 1 && nums[tail] == curTail )
						--tail;

					if ( nums[head] == curHead && nums[tail] == curTail )
					{
						break;
					}
				}
			}
		}

		return results;
	}

private:

	void QuickSort( vector<int>& nums , int start , int end )
	{
		if ( start == end )
			return;

		int key = start;
		int keyVal = nums[start];
		int head = start;
		int tail = end;
		while ( head < tail )
		{
			while ( tail > key && nums[tail] >= keyVal )
			{
				--tail;
			}
			if ( tail > key )
			{
				swapValues( nums[key] , nums[tail] );
				key = tail;
			}
			
			while ( head < key && nums[head] <= keyVal )
			{
				++head;
			}
			if ( head < key )
			{
				swapValues( nums[key] , nums[head] );
				key = head;
			}	
		}

		if ( start < key - 1 )
			QuickSort( nums , start , key - 1 );

		if ( key + 1 < end )
			QuickSort( nums , key + 1 , end );
	}

	void swapValues( int &a , int &b )
	{
		int temp = a;
		a = b;
		b = temp;
	}

};

//int main()
//{
//	//{ 5,1,7,-15,56,2,-7 };
//	//{ -1, 0, 1, 2, -1, -4 };
//	//{ -1,0,0,0,0,1 };
//	//{ 5,1,7,-15,56,2,-7 };
//	vector<int> input = { -1,0,0,0,0,1 };
//	ThreeSum test;
//	vector<vector<int>> results = test.threeSum( input );
//	
//	int numOfGroups = results.size();
//	for ( int i = 0; i < numOfGroups; ++i )
//	{
//		cout << results[i][0] << ' ' << results[i][1] << ' ' << results[i][2] << endl;
//	}
//
//	system( "pause" );
//	return 0;
//}