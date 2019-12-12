/*
ʵ�ֻ�ȡ��һ�����еĺ������㷨��Ҫ���������������������г��ֵ�������һ����������С�
�����������һ����������У��������������г���С�����У����������У���
����ԭ���޸ģ�ֻ����ʹ�ö��ⳣ���ռ䡣

������һЩ���ӣ�����λ������У�����Ӧ���λ���Ҳ��С�
1,2,3 �� 1,3,2
3,2,1 �� 1,2,3
1,1,5 �� 1,5,1

��Դ�����ۣ�LeetCode��
���ӣ�https://leetcode-cn.com/problems/next-permutation
����Ȩ������������С���ҵת������ϵ�ٷ���Ȩ������ҵת����ע��������
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

		//�Ӻ���ǰ�ң��ҵ�һ�����������index
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

		//����ҵ��ˣ��ٷ���Ѱ�ң����򣩣�Ѱ�ҽ������������Ȼ�󽻻�����������������foundIndex֮�����
		//���û�ҵ�����������ȫ������
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