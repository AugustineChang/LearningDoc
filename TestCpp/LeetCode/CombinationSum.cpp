/*
给定一个无重复元素的数组 candidates 和一个目标数 target ，找出 candidates 中所有可以使数字和为 target 的组合。
candidates 中的数字可以无限制重复被选取。

说明：
所有数字（包括 target）都是正整数。
解集不能包含重复的组合。 

示例 1:
输入: candidates = [2,3,6,7], target = 7,
所求解集为:
[
  [7],
  [2,2,3]
]

示例 2:
输入: candidates = [2,3,5], target = 8,
所求解集为:
[
  [2,2,2,2],
  [2,3,3],
  [3,5]
]

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/combination-sum
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

/*
给定一个数组 candidates 和一个目标数 target ，找出 candidates 中所有可以使数字和为 target 的组合。

candidates 中的每个数字在每个组合中只能使用一次。

说明：

所有数字（包括目标数）都是正整数。
解集不能包含重复的组合。 
示例 1:

输入: candidates = [10,1,2,7,6,1,5], target = 8,
所求解集为:
[
  [1, 7],
  [1, 2, 5],
  [2, 6],
  [1, 1, 6]
]
示例 2:

输入: candidates = [2,5,2,1,2], target = 5,
所求解集为:
[
  [1,2,2],
  [5]
]

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/combination-sum-ii
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <vector>
#include <iostream>
using namespace std;

class CombinationSum
{
public:
	vector<vector<int>> combinationSum( vector<int>& candidates, int target ) 
	{
		vector<vector<int>> results;
		int numOfList = candidates.size();
		if( numOfList <= 0 )
			return results;

		QuickSort( candidates, 0, numOfList - 1 );

		vector<int> path;
		path.reserve( numOfList );
		searchRecursively( candidates, target, 0, path, results );

		return results;
	}

	vector<vector<int>> combinationSum2( vector<int>& candidates, int target )
	{
		vector<vector<int>> results;
		int numOfList = candidates.size();
		if( numOfList <= 0 )
			return results;

		QuickSort( candidates, 0, numOfList - 1 );

		vector<int> path;
		path.reserve( numOfList );
		searchRecursively2( candidates, target, 0, path, results );

		return results;
	}

private:

	void searchRecursively( vector<int>& candidates , int target , int start , vector<int>& path , vector<vector<int>>& results )
	{
		if( target < 0 )
			return;
		if( target == 0 )
		{
			results.push_back( path );
			return;
		}

		int numOfList = candidates.size();
		for( int i = start; i < numOfList; ++i )
		{
			int candi = candidates[i];
			path.push_back( candi );
			searchRecursively( candidates, target - candi, i, path, results );
			path.pop_back();
		}
	}

	void searchRecursively2( vector<int>& candidates, int target, int start, vector<int>& path, vector<vector<int>>& results )
	{
		if( target < 0 )
			return;
		if( target == 0 )
		{
			results.push_back( path );
			return;
		}

		int numOfList = candidates.size();
		int last = -1;
		for( int i = start; i < numOfList; ++i )
		{
			int candi = candidates[i];
			if ( candi == last )
				continue;
			last = candi;

			path.push_back( candi );
			searchRecursively2( candidates, target - candi, i + 1, path, results );
			path.pop_back();
		}
	}

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
//	//{ 2,3,6,7 };
//	//{ 10,1,2,7,6,1,5 };
//	vector<int> input = { 2,3,5 };
//	vector<int> input2 = { 2,5,2,1,2 };
//	CombinationSum test;
//	//vector<vector<int>> results = test.combinationSum( input, 8 );
//	vector<vector<int>> results = test.combinationSum2( input2, 5 );
//	
//	int numOfGroups = results.size();
//	for ( int i = 0; i < numOfGroups; ++i )
//	{
//		for( auto iter = results[i].begin(); iter != results[i].end(); ++iter )
//		{
//			cout << *iter << ' ';
//		}
//		cout << endl;
//	}
//
//	system( "pause" );
//	return 0;
//}