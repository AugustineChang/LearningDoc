/*
实现获取下一个排列的函数，算法需要将给定数字序列重新排列成字典序中下一个更大的排列。
如果不存在下一个更大的排列，则将数字重新排列成最小的排列（即升序排列）。
必须原地修改，只允许使用额外常数空间。

以下是一些例子，输入位于左侧列，其相应输出位于右侧列。
1,2,3 → 1,3,2
3,2,1 → 1,2,3
1,1,5 → 1,5,1

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/next-permutation
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <vector>
#include <iostream>
using namespace std;

class NextPermutation 
{
public:
	void nextPermutation( vector<int>& nums ) 
	{
		int numSize = nums.size();
		if( numSize <= 1 )
			return;

		//从后往前找，找第一个不是逆序的index
		int lastNum = nums[numSize - 1];
		int foundIndex = -1;
		for( int i = numSize - 2; i >= 0; --i )
		{
			int curNum = nums[i];
			if( curNum < lastNum )
			{
				foundIndex = i;
				break;
			}
			lastNum = curNum;
		}

		//如果找到了，再反向寻找（正向），寻找仅比它大的数，然后交换这俩数，正序排列foundIndex之后的数
		//如果没找到，正序排列全部的数
		if( foundIndex >= 0 )
		{
			int target = nums[foundIndex];
			int nextIndex = foundIndex + 1;
			for( int i = foundIndex + 2; i < numSize; ++i )
			{
				int curNum = nums[i];
				if( curNum <= target )
				{
					break;
				}
				nextIndex = i;
			}

			//swap
			swap( nums, foundIndex, nextIndex );
		}
		
		revert( nums, foundIndex + 1 );
	}

private:

	void revert( vector<int>& nums, int startIndex)
	{
		int numSize = nums.size();
		int half = ( numSize - startIndex ) / 2;
		for( int i = 0; i < half; ++i )
		{
			swap( nums, startIndex + i, numSize - 1 - i );
		}
	}

	void swap( vector<int>& nums, int ia, int ib )
	{
		int temp = nums[ia];
		nums[ia] = nums[ib];
		nums[ib] = temp;
	}
};

//int main()
//{
//	vector<int> input = { 1,4,3,2 };
//
//	NextPermutation test;
//	test.nextPermutation( input );
//
//	for( int val : input )
//	{
//		cout << val << ' ';
//	}
//	cout << endl;
//
//	system( "pause" );
//	return 0;
//}