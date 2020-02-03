/*
给定一个未排序的整数数组，找出其中没有出现的最小的正整数。

示例 1:
输入: [1,2,0]
输出: 3

示例 2:
输入: [3,4,-1,1]
输出: 2

示例 3:
输入: [7,8,9,11,12]
输出: 1

说明:
你的算法的时间复杂度应为O(n)，并且只能使用常数级别的空间。

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/first-missing-positive
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <vector>
#include <iostream>
using namespace std;

class FirstMissingPositive
{
public:
	int firstMissingPositive( vector<int>& nums ) 
	{
		int numOfList = nums.size();
		for( int i = 0; i < numOfList; ++i )
		{
			int val = nums[i];
			if( val != i + 1 )
			{
				if( val >= 1 && val <= numOfList )
				{
					if( val != nums[val - 1] )
					{
						swapValues( nums[i], nums[val - 1] );
						--i;
					}
				}
			}
		}

		for( int i = 0; i < numOfList; ++i )
		{
			if( nums[i] != i + 1 )
			{
				return i + 1;
			}
		}
		return numOfList + 1;
	}

private:
	void swapValues( int &a, int &b )
	{
		int temp = a;
		a = b;
		b = temp;
	}
};

//int main()
//{
//	vector<int> input = { 2,2,2 };
//	FirstMissingPositive test;
//	cout << test.firstMissingPositive( input ) << endl;
//
//	system( "pause" );
//	return 0;
//}