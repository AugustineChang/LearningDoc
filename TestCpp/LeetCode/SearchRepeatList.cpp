/*
给定一个按照升序排列的整数数组 nums，和一个目标值 target。找出给定目标值在数组中的开始位置和结束位置。
你的算法时间复杂度必须是 O(log n) 级别。
如果数组中不存在目标值，返回 [-1, -1]。

示例 1:
输入: nums = [5,7,7,8,8,10], target = 8
输出: [3,4]

示例 2:
输入: nums = [5,7,7,8,8,10], target = 6
输出: [-1,-1]

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/find-first-and-last-position-of-element-in-sorted-array
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <vector>
#include <iostream>
using namespace std;

class SearchRepeatList 
{
public:
	vector<int> searchRange( vector<int>& nums, int target ) 
	{
		vector<int> output( 2, -1 );

		int numSize = nums.size();
		if( numSize <= 0 )
			return output;

		int start = 0;
		int end = numSize - 1;

		//find first
		while( start + 1 < end )
		{
			int half = ( start + end ) / 2;
			int halfVal = nums[half];

			if( target <= halfVal )//选左端
			{
				end = half;
			}
			else//选右端
			{
				start = half;
			}
		}

		if( nums[start] == target )
		{
			output[0] = start;
		}
		else if( nums[end] == target )
		{
			output[0] = end;
		}
		else
		{
			return output;
		}
		
		start = 0;
		end = numSize - 1;

		//find last
		while( start + 1 < end )
		{
			int half = ( start + end ) / 2;
			int halfVal = nums[half];

			if( target < halfVal )//选左端
			{
				end = half;
			}
			else//选右端
			{
				start = half;
			}
		}

		if( nums[end] == target )
		{
			output[1] = end;
		}
		else if( nums[start] == target )
		{
			output[1] = start;
		}
		return output;
	}
};

//int main()
//{
//	SearchRepeatList test;
//
//	vector<int> input = { 8,8 };
//	const vector<int> &output = test.searchRange( input, 8 );
//
//	cout << '[' << output[0] << ',' << output[1] << ']' << endl;
//
//	system( "pause" );
//	return 0;
//}