/*
将两个有序链表合并为一个新的有序链表并返回。新链表是通过拼接给定的两个链表的所有节点组成的。 
示例：

输入：1->2->4, 1->3->4
输出：1->1->2->3->4->4

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/merge-two-sorted-lists
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <iostream>
#include <vector>
#include "Common.h"
using namespace std;

class MergeLists 
{
public:
	ListNode* mergeTwoLists( ListNode* l1, ListNode* l2 )
	{
		if( l1 == nullptr && l2 == nullptr )
			return nullptr;
		else if( l1 == nullptr )
			return l2;
		else if( l2 == nullptr )
			return l1;

		ListNode *head = nullptr;
		ListNode *curPtr = nullptr;

		ListNode *list1Ptr = l1;
		ListNode *list2Ptr = l2;

		while( list1Ptr != nullptr || list2Ptr != nullptr )
		{
			if( list1Ptr != nullptr && list2Ptr != nullptr )
			{
				ListNode *&targetPtr = list1Ptr->val < list2Ptr->val ? list1Ptr : list2Ptr;

				if( curPtr == nullptr )
				{
					curPtr = targetPtr;
					head = targetPtr;
				}
				else
				{
					curPtr->next = targetPtr;
					curPtr = targetPtr;
				}

				targetPtr = targetPtr->next;
			}
			else if( list1Ptr != nullptr )
			{
				curPtr->next = list1Ptr;
				curPtr = list1Ptr;
				list1Ptr = list1Ptr->next;
			}
			else if( list2Ptr != nullptr )
			{
				curPtr->next = list2Ptr;
				curPtr = list2Ptr;
				list2Ptr = list2Ptr->next;
			}
		}

		return head;
	}

	ListNode* mergeKLists( vector<ListNode*>& lists ) 
	{
		int numOfK = lists.size();
		if( numOfK <= 0 )
			return nullptr;

		while( numOfK > 1 )
		{
			int halfK = numOfK / 2;
			for( int i = 0; i < halfK; ++i )
			{
				lists[i] = mergeTwoLists( lists[i * 2], lists[i * 2 + 1] );
			}
			if( halfK * 2 < numOfK )
			{
				lists[halfK] = lists[numOfK - 1];
				++halfK;
			}
			numOfK = halfK;
		}
		

		return lists[0];
	}
};

//int main()
//{
//	ListNode a1( 1 );
//	ListNode a2( 3 );
//	ListNode a3( 7 );
//	ListNode a4( 11 );
//	ListNode a5( 20 );
//
//	a1.next = &a2;
//	a2.next = &a3;
//	a3.next = &a4;
//	a4.next = &a5;
//
//	ListNode b1( 2 );
//	ListNode b2( 5 );
//	ListNode b3( 6 );
//	ListNode b4( 12 );
//	ListNode b5( 15 );
//
//	b1.next = &b2;
//	b2.next = &b3;
//	b3.next = &b4;
//	b4.next = &b5;
//
//	MergeLists test;
//
//	ListNode* output = test.mergeTwoLists( &a1, &b1 );
//	while( output != nullptr )
//	{
//		cout << output->val << "->";
//		output = output->next;
//	}
//	cout << endl;
//
//	system( "pause" );
//	return 0;
//}


//int main()
//{
//
//	int a[5] = { 2,5,12,67,100 };
//	int b[3] = { 8,23,24 };
//	int c[7] = { 1,6,10,11,15,17,50 };
//
//	vector<ListNode*> input;
//	input.reserve( 3 );
//	input.push_back( createList( a, 5 ) );
//	input.push_back( createList( b, 3 ) );
//	input.push_back( createList( c, 7 ) );
//
//	MergeLists test;
//	ListNode* output = test.mergeKLists( input );
//
//	while( output != nullptr )
//	{
//		cout << output->val << "->";
//		output = output->next;
//	}
//	cout << endl;
//
//	system( "pause" );
//	return 0;
//}