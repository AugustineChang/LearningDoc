/*
给出两个 非空 的链表用来表示两个非负的整数。其中，它们各自的位数是按照 逆序 的方式存储的，并且它们的每个节点只能存储 一位 数字。
如果，我们将这两个数相加起来，则会返回一个新的链表来表示它们的和。
您可以假设除了数字 0 之外，这两个数都不会以 0 开头。

示例：

输入：(2 -> 4 -> 3) + (5 -> 6 -> 4)
输出：7 -> 0 -> 8
原因：342 + 465 = 807

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/add-two-numbers
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <iostream>
#include <vector>
#include "Common.h"
using namespace std;

class AddTwoNumbers 
{
public:

	ListNode* addTwoNumbers( ListNode* l1 , ListNode* l2 ) 
	{
		ListNode* l1ptr = l1;
		ListNode* l2ptr = l2;
		ListNode* output = new ListNode( 0 );
		ListNode* outputPtr = output;

		int carry = 0;
		while ( l1ptr != nullptr || l2ptr != nullptr )
		{
			int sum = carry;
			if ( l1ptr != nullptr )
			{
				sum += l1ptr->val;
				l1ptr = l1ptr->next;
			}

			if ( l2ptr != nullptr )
			{
				sum += l2ptr->val;
				l2ptr = l2ptr->next;
			}

			carry = sum / 10;
			sum = sum % 10;
			outputPtr->next = new ListNode( sum );
			outputPtr = outputPtr->next;
		}

		if ( carry > 0 )
		{
			outputPtr->next = new ListNode( carry );
		}

		return output->next;
	}
};

//int main()
//{
//	vector<int> inputA = { 1 , 2,  3 , 4 , 5 , 6 };
//	vector<int> inputB = { 9 , 1 , 9 };
//	
//	ListNode* l1 = nullptr;
//	ListNode* l1ptr = nullptr;
//	ListNode* l2 = nullptr;
//	ListNode* l2ptr = nullptr;
//	int numA = inputA.size();
//	int numB = inputB.size();
//	for ( int i = numA - 1; i >= 0; --i )
//	{
//		if ( l1 == nullptr )
//		{
//			l1 = new ListNode( inputA[i] );
//			l1ptr = l1;
//		}
//		else
//		{
//			l1ptr->next = new ListNode( inputA[i] );
//			l1ptr = l1ptr->next;
//		}
//	}
//	for ( int i = numB - 1; i >= 0; --i )
//	{
//		if ( l2 == nullptr )
//		{
//			l2 = new ListNode( inputB[i] );
//			l2ptr = l2;
//		}
//		else
//		{
//			l2ptr->next = new ListNode( inputB[i] );
//			l2ptr = l2ptr->next;
//		}
//	}
//
//	AddTwoNumbers test;
//	ListNode* output = test.addTwoNumbers( l1 , l2 );
//
//	while ( output != nullptr )
//	{
//		cout << output->val << ' ';
//		output = output->next;
//	}
//	cout << endl;
//
//	system( "Pause" );
//	return 0;
//}