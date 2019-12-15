/*
给定一个排序数组和一个目标值，在数组中找到目标值，并返回其索引。如果目标值不存在于数组中，返回它将会被按顺序插入的位置。
你可以假设数组中无重复元素。

示例 1:
输入: [1,3,5,6], 5
输出: 2

示例 2:
输入: [1,3,5,6], 2
输出: 1

示例 3:
输入: [1,3,5,6], 7
输出: 4

示例 4:
输入: [1,3,5,6], 0
输出: 0

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/search-insert-position
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <vector>
#include <iostream>
using namespace std;

class SimpleSearch 
{
public:
	int searchInsert( vector<int>& nums, int target ) 
	{
		int numSize = nums.size();
		if( numSize <= 0 )
			return 0;

		int start = 0;
		int end = numSize - 1;
		
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

		if( target <= nums[start])
		{
			return start;
		}
		else if( target <= nums[end] )
		{
			return end;
		}
		else
		{
			return end + 1;
		}
	}
};

//int main()
//{
//	SimpleSearch test;
//
//	vector<int> input = { 1,3 };
//	cout << test.searchInsert( input, 0 ) << endl;
//
//	system( "pause" );
//	return 0;
//}