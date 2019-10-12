/*
给定两个大小为 m 和 n 的有序数组 nums1 和 nums2。
请你找出这两个有序数组的中位数，并且要求算法的时间复杂度为 O(log(m + n))。
你可以假设 nums1 和 nums2 不会同时为空。

示例 1:
nums1 = [1, 3]
nums2 = [2]

则中位数是 2.0

示例 2:
nums1 = [1, 2]
nums2 = [3, 4]

则中位数是 (2 + 3)/2 = 2.5

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/median-of-two-sorted-arrays
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <vector>
#include <iostream>
using namespace std;

class FindMedian 
{
public:
	double findMedianSortedArrays( vector<int>& nums1 , vector<int>& nums2 ) 
	{
		int len1 = nums1.size();
		int len2 = nums2.size();

		//寻找第k小的数
		bool isEven = ( len1 + len2 ) % 2 == 0;
		int k = ( len1 + len2 ) / 2;
		k += isEven ? 0 : 1;

		int l1Ptr = 0;
		int l2Ptr = 0;

		while ( k > 1 )
		{
			if ( l1Ptr < len1 && l2Ptr < len2 )
			{
				int halfK = k / 2;
				int l1Test = l1Ptr + halfK - 1;
				int l2Test = l2Ptr + halfK - 1;
				if ( l1Test >= len1 )
					l1Test = len1 - 1;
				if ( l2Test >= len2 )
					l2Test = len2 - 1;

				if ( nums1[l1Test] < nums2[l2Test] )
				{
					k -= ( l1Test - l1Ptr + 1 );
					l1Ptr = l1Test + 1;
				}
				else
				{
					k -= ( l2Test - l2Ptr + 1 );
					l2Ptr = l2Test + 1;
				}
			}
			else
				break;
		}

		if ( l1Ptr < len1 && l2Ptr < len2 )
		{
			bool isNum1Smaller = nums1[l1Ptr] < nums2[l2Ptr];
			if ( isEven )
			{
				double firstNum = isNum1Smaller ? nums1[l1Ptr] : nums2[l2Ptr];

				double secondNum = 0.0f;
				if ( isNum1Smaller )
				{
					if ( l1Ptr + 1 < len1 )
					{
						secondNum = nums2[l2Ptr] < nums1[l1Ptr + 1] ? nums2[l2Ptr] : nums1[l1Ptr + 1];
					}
					else
					{
						secondNum = nums2[l2Ptr];
					}
				}
				else
				{
					if ( l2Ptr + 1 < len2 )
					{
						secondNum = nums1[l1Ptr] < nums2[l2Ptr + 1] ? nums1[l1Ptr] : nums2[l2Ptr + 1];
					}
					else
					{
						secondNum = nums1[l1Ptr];
					}
				}

				return ( firstNum + secondNum ) / 2.0;
			}
			else
				return isNum1Smaller ? nums1[l1Ptr] : nums2[l2Ptr];
		}
		else if ( l1Ptr < len1 )
		{
			if ( isEven )
				return ( nums1[l1Ptr + k - 1] + nums1[l1Ptr + k] ) / 2.0;
			else
				return nums1[l1Ptr + k - 1];
		}
		else if ( l2Ptr < len2 )
		{
			if ( isEven )
				return ( nums2[l2Ptr + k - 1] + nums2[l2Ptr + k] ) / 2.0;
			else
				return nums2[l2Ptr + k - 1];
		}
		else
			return 0.0;
	}
};

//int main()
//{
//	vector<int> inputA = { 1, 2 , 3 };
//	vector<int> inputB = { 5, 7 , 9 , 10 ,11, 12 };
//
//	FindMedian test;
//	cout << test.findMedianSortedArrays( inputA , inputB ) << endl;
//
//	system( "pause" );
//	return 0;
//}