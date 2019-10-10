/*
给定一个整数数组 nums 和一个目标值 target，请你在该数组中找出和为目标值的那 两个 整数，并返回他们的数组下标。
你可以假设每种输入只会对应一个答案。但是，你不能重复利用这个数组中同样的元素。

示例:
给定 nums = [2, 7, 11, 15], target = 9

因为 nums[0] + nums[1] = 2 + 7 = 9
所以返回 [0, 1]

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/two-sum
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <iostream>
#include <vector>
using namespace std;

class TwoSum 
{
	struct ListNode
	{
		ListNode()
			: oldIndex( 0 ) , value( 0 )
		{}

		ListNode( int id , int val ) 
			: oldIndex( id ) , value( val )
		{}

		int oldIndex;
		int value;
	};

public:

	vector<int> twoSum( vector<int>& nums , int target ) 
	{
		int numOfInput = nums.size();
		vector<ListNode> inputList, sortedList;
		inputList.reserve( numOfInput );
		sortedList.reserve( numOfInput );
		for ( int i = 0; i < numOfInput; ++i )
		{
			inputList.push_back( ListNode( i , nums[i] ) );
		}
		MergeSort( inputList , 0 , numOfInput - 1 , sortedList );
		inputList.clear();

		vector<int> result;
		for ( int i = 0; i < numOfInput; ++i )
		{
			int foundIndex = binarySearch( sortedList , target - nums[i] );
			if ( foundIndex >= 0 )
			{
				int realIndex = sortedList[foundIndex].oldIndex;
				if ( realIndex == i )
					continue;

				result.push_back( i );
				result.push_back( realIndex );
				break;
			}
		}
		return result;
	}

private:
	
	void MergeSort( const vector<ListNode>& list , int iStart , int iEnd , vector<ListNode> &outList )
	{
		if ( iStart == iEnd )
		{
			outList.push_back( list[iStart] );
			return;
		}
		
		int iMid = ( iStart + iEnd ) / 2;
		vector<ListNode> out1;
		out1.reserve( iMid - iStart + 1 );
		vector<ListNode> out2;
		out2.reserve( iEnd - iMid );

		MergeSort( list , iStart , iMid , out1 );
		MergeSort( list , iMid + 1 , iEnd , out2 );

		Merge( out1 , out2 , outList );
	}

	void Merge( const vector<ListNode>& listA , const vector<ListNode>& listB , vector<ListNode>& output )
	{
		int lenA = listA.size();
		int lenB = listB.size();
		int newLen = lenA + lenB;

		int totalLen = lenA + lenB;
		int iA = 0;
		int iB = 0;
		for ( int i = 0; i < totalLen; ++i )
		{
			if ( iA < lenA && iB < lenB )
			{
				if ( listA[iA].value < listB[iB].value )
				{
					output.push_back( listA[iA] );
					++iA;
				}
				else
				{
					output.push_back( listB[iB] );
					++iB;
				}
			}
			else if ( iA < lenA )
			{
				output.push_back( listA[iA] );
				++iA;
			}
			else if ( iB < lenB )
			{
				output.push_back( listB[iB] );
				++iB;
			}
			else
				break;
		}
	}

	//list 需要升序
	int binarySearch( const vector<ListNode>& list , int value )
	{
		int numOfList = list.size();
		int start = 0;
		int end = numOfList - 1;
		while ( start != end )
		{
			int midIndex = ( start + end ) / 2;
			if ( list[midIndex].value == value )
			{
				return midIndex;
			}
			else if ( value < list[midIndex].value )
			{
				end = midIndex;
			}
			else
			{
				start = midIndex + 1;
			}
		}
		return list[start].value == value ? start : -1;
	}
};

int main()
{
	//vector<int> input = { 2 , 15 , -5 , 7 , 4 , 90 , 32 , 6 , -3 , -58 , 5 };
	vector<int> input = { 3 , 2 , 4 };
	//const vector<int>& output = test.twoSum( input , 4 );
	
	TwoSum test;
	vector<int> output = test.twoSum( input , 6 );

	cout << output[0] << ' ' << output[1] << endl;

	system( "Pause" );
	return 0;
}