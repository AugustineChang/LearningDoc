/*
给定一个包含 n 个整数的数组 nums 和一个目标值 target，判断 nums 中是否存在四个元素 a，b，c 和 d ，使得 a + b + c + d 的值与 target 相等？找出所有满足条件且不重复的四元组。

注意：
答案中不可以包含重复的四元组。

示例：
给定数组 nums = [1, 0, -1, 0, -2, 2]，和 target = 0。

满足要求的四元组集合为：
[
  [-1,  0, 0, 1],
  [-2, -1, 1, 2],
  [-2,  0, 0, 2]
]

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/4sum
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <vector>
#include <iostream>
using namespace std;

class FourSum
{
public:
	vector<vector<int>> fourSum( vector<int>& nums, int target ) 
	{
		vector<vector<int>> output;
		int numSize = nums.size();
		if( numSize <= 3 )
			return output;

		QuickSort( nums, 0, numSize - 1 );

		int i1Size = numSize - 3;
		int i2Size = numSize - 2;
		int lastI1 = -1;
		int lastI2 = -1;

		for( int i1 = 0; i1 < i1Size; ++i1 )
		{
			if( lastI1 >= 0 && nums[i1] <= nums[lastI1] )
				continue;
			lastI1 = i1;

			lastI2 = -1;
			for( int i2 = i1 + 1; i2 < i2Size; ++i2 )
			{
				if( lastI2 >= 0 && nums[i2] <= nums[lastI2] )
					continue;
				lastI2 = i2;

				int i3 = i2 + 1;
				int i4 = numSize - 1;

				while( true )
				{
					int sum = nums[i1] + nums[i2] + nums[i3] + nums[i4];
					if( sum < target )
					{
						int curVal = nums[i3];
						while( i3 < i4 - 1 && nums[i3] <= curVal )
							++i3;

						if ( nums[i3] <= curVal )
							break;
					}
					else if( sum > target )
					{
						int curVal = nums[i4];
						while( i4 > i3 + 1 && nums[i4] >= curVal )
							--i4;

						if( nums[i4] >= curVal )
							break;
					}
					else
					{
						vector<int> oneRet;
						oneRet.reserve( 4 );

						oneRet.push_back( nums[i1] );
						oneRet.push_back( nums[i2] );
						oneRet.push_back( nums[i3] );
						oneRet.push_back( nums[i4] );

						output.push_back( oneRet );

						int curi3 = nums[i3];
						while( i3 < i4 - 1 && nums[i3] <= curi3 )
							++i3;

						int curi4 = nums[i4];
						while( i4 > i3 + 1 && nums[i4] >= curi4 )
							--i4;

						if( nums[i3] <= curi3 && nums[i4] >= curi4 )
							break;
					}
				}
			}
		}

		return output;
	}

private:

	void QuickSort( vector<int>& nums, int start, int end )
	{
		if( start == end )
			return;

		int key = start;
		int keyVal = nums[start];
		int head = start;
		int tail = end;
		while( head < tail )
		{
			while( tail > key && nums[tail] >= keyVal )
			{
				--tail;
			}
			if( tail > key )
			{
				swapValues( nums[key], nums[tail] );
				key = tail;
			}

			while( head < key && nums[head] <= keyVal )
			{
				++head;
			}
			if( head < key )
			{
				swapValues( nums[key], nums[head] );
				key = head;
			}
		}

		if( start < key - 1 )
			QuickSort( nums, start, key - 1 );

		if( key + 1 < end )
			QuickSort( nums, key + 1, end );
	}

	void swapValues( int &a, int &b )
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
//	vector<int> input = { -1,-5,-5,-3,2,5,0,4 };
//	FourSum test;
//	vector<vector<int>> results = test.fourSum( input , -7 );
//
//	int numOfGroups = results.size();
//	for( int i = 0; i < numOfGroups; ++i )
//	{
//		cout << results[i][0] << ' ' << results[i][1] << ' ' << results[i][2] << ' ' << results[i][3] << endl;
//	}
//
//	system( "pause" );
//	return 0;
//}