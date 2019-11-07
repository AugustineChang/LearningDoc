/*
给定一个包括 n 个整数的数组 nums 和 一个目标值 target。找出 nums 中的三个整数，使得它们的和与 target 最接近。返回这三个数的和。假定每组输入只存在唯一答案。

例如，给定数组 nums = [-1，2，1，-4], 和 target = 1.
与 target 最接近的三个数的和为 2. (-1 + 2 + 1 = 2).

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/3sum-closest
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <vector>
#include <iostream>
using namespace std;

class ThreeSumClosest
{
public:
	int threeSumClosest( vector<int>& nums , int target )
	{
		int numOfList = nums.size();
		if ( numOfList <= 0 )
			return 0;

		QuickSort( nums , 0 , numOfList - 1 );

		int minDiff = INT_MAX;
		int resultSum = 0;
		//int indices[3];
		for ( int i = 0; i < numOfList - 2; ++i )
		{
			if ( i > 0 && nums[i-1] == nums[i] )
				continue;

			int head = i + 1;
			int tail = numOfList - 1;
			while ( true )
			{
				int sum = nums[i] + nums[head] + nums[tail];
				if ( sum <= target )
				{
					int curDiff = target - sum;
					if ( curDiff < minDiff )
					{
						resultSum = sum;
						minDiff = curDiff;

						//indices[0] = i;
						//indices[1] = head;
						//indices[2] = tail;
					}

					int curVal = nums[head];
					while ( head < tail - 1 && nums[head] == curVal )
						++head;

					if ( nums[head] == curVal )
						break;
				}
				else //if ( sum > target )
				{
					int curDiff = sum - target;
					if ( curDiff < minDiff )
					{
						resultSum = sum;
						minDiff = curDiff;

						//indices[0] = i;
						//indices[1] = head;
						//indices[2] = tail;
					}

					int curVal = nums[tail];
					while ( tail > head + 1 && nums[tail] == curVal )
						--tail;
					
					if ( nums[tail] == curVal )
						break;
				}
			}
		}

		//cout << resultSum << "= (" << indices[0] << "+" << indices[1] << "+" << indices[2] << ")" << endl;
		return resultSum;
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

int main()
{
	//{ 5,1,7,-15,56,2,-7 };
	//{ -1, 0, 1, 2, -1, -4 };
	//{ -1,0,0,0,0,1 };
	//{ 5,1,7,-15,56,2,-7 };
	vector<int> input = { -1,2,1,4 };
	ThreeSumClosest test;
	test.threeSumClosest( input , 0 );

	system( "pause" );
	return 0;
}