/*
假设按照升序排序的数组在预先未知的某个点上进行了旋转。
( 例如，数组 [0,1,2,4,5,6,7] 可能变为 [4,5,6,7,0,1,2] )。
搜索一个给定的目标值，如果数组中存在这个目标值，则返回它的索引，否则返回 -1 。
你可以假设数组中不存在重复的元素。
你的算法时间复杂度必须是 O(log n) 级别。

示例 1:
输入: nums = [4,5,6,7,0,1,2], target = 0
输出: 4

示例 2:
输入: nums = [4,5,6,7,0,1,2], target = 3
输出: -1

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/search-in-rotated-sorted-array
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <vector>
#include <iostream>
using namespace std;

class SearchBrokenList 
{
public:
	int search( vector<int>& nums, int target ) 
	{
		int numSize = nums.size();
		if( numSize <= 0 )
			return -1;

		int start = 0;
		int end = numSize - 1;

		while( start != end )
		{
			int half = ( start + end ) / 2;
			int halfVal = nums[half];
			int startVal = nums[start];
			int endVal = nums[end];

			if( startVal < endVal )// 正常的折半查找
			{
				if( target <= halfVal )//选左端
				{
					end = half;
				}
				else//选右端
				{
					start = half + 1;
				}
			}
			else
			{
				if( halfVal >= startVal )
				{
					if( target <= endVal || target > halfVal )//选右端
					{
						start = half + 1;
					}
					else if( startVal <= target && target <= halfVal )//选左端
					{
						end = half;
					}
					else // endVal > target && target > startVal 这段没有值 反-1
						return -1;
				}
				else
				{
					if( target <= halfVal || target >= startVal )//选左端
					{
						end = half;
					}
					else if( halfVal < target && target <= endVal )//选右端
					{
						start = half + 1;
					}
					else // endVal > target && target > startVal 这段没有值 反-1
						return -1;
				}
			}
		}
		
		return nums[start] == target ? start : -1;
	}
};

//int main()
//{
//	SearchBrokenList test;
//	vector<int> input = { 3,1 };//{ 49, 66, 105, 151, 1, 3, 6, 12, 15, 16, 25, 37 };
//
//	cout << test.search( input, 1 ) << endl;
//
//	system( "pause" );
//	return 0;
//}