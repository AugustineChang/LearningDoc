/*Given an array of integers , find two numbers such that they add up to a specific target number.

The function twoSum should return indices of the two numbers such that they add up to the target , where index1 must be less than index2.Please note that your returned answers( both index1 and index2 ) are not zero - based.

You may assume that each input would have exactly one solution.

Input: numbers = { 2 , 7 , 11 , 15 } , target = 9
Output : index1 = 1 , index2 = 2*/

#include<vector>

using namespace std;

class Solution
{
public:
	struct MyStruct
	{
		int originIndex;
		int value;
	};


	vector<int> twoSum( vector<int> &numbers , int target )
	{
		vector<int> result;
		vector<MyStruct> temp;

		size_t length = numbers.size();
		for( size_t i = 0; i < length; i++ )
		{
			MyStruct one;
			one.originIndex = i;
			one.value = target - numbers[i];

			int ret = halfFind( temp , one );
			if( ret == -1 )
			{
				one.value = numbers[i];
				halfInsert( temp , one );
			}
			else
			{
				result.push_back( ret + 1 );
				result.push_back( i + 1 );
			}
		}
		return result;
	}

	void halfInsert( vector<MyStruct>& findCol , MyStruct newNum )
	{
		if( findCol.size() == 0 )
		{
			findCol.push_back( newNum );
		}
		else if( findCol.size() == 1 )
		{
			if( findCol[0].value <= newNum.value )
			{
				findCol.insert( findCol.begin() + 1 , newNum );
			}
			else
			{
				findCol.insert( findCol.begin() , newNum );
			}
		}
		else
		{
			int startIndex = -1;
			int endIndex = findCol.size();
			while( true )
			{
				int middle = ( startIndex + endIndex ) / 2;

				if( findCol[middle].value <= newNum.value )
				{
					startIndex = middle;
				}
				else
				{
					endIndex = middle;
				}

				if( endIndex - startIndex == 1 )
				{
					findCol.insert( findCol.begin() + endIndex , newNum );
					break;
				}
			}
		}
	}

	int halfFind( vector<MyStruct>& findCol , MyStruct findNum )
	{
		if( findCol.size() == 0 )
		{
			return -1;
		}
		else if( findCol.size() == 1 )
		{
			if( findCol[0].value == findNum.value )
			{
				return findCol[0].originIndex;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			int startIndex = -1;
			int endIndex = findCol.size();
			while( true )
			{
				int middle = ( startIndex + endIndex ) / 2;

				if( findCol[middle].value < findNum.value )
				{
					startIndex = middle;
				}
				else if( findCol[middle].value > findNum.value )
				{
					endIndex = middle;
				}
				else
				{
					return  findCol[middle].originIndex;
				}

				if( endIndex - startIndex == 1 )
				{
					return -1;
				}
			}
		}
	}
};



/*int main()
{
	Solution test;

	vector<int> input = { 2 , 15 , -5 , 7 , 4 , 90 , 32 , 6 , -3 , -58 , 5 };
	const vector<int>& output = test.twoSum( input , 4 );

	return 0;
}*/