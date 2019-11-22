/*
给定一个链表，删除链表的倒数第 n 个节点，并且返回链表的头结点。

示例：
给定一个链表: 1->2->3->4->5, 和 n = 2.
当删除了倒数第二个节点后，链表变为 1->2->3->5.

说明：
给定的 n 保证是有效的。

进阶：
你能尝试使用一趟扫描实现吗？

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/remove-nth-node-from-end-of-list
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <iostream>
#include "Common.h"
using namespace std;

class DeleteLinkNode 
{
public:

	ListNode* removeNthFromEnd( ListNode* head , int n ) 
	{
		if ( head == nullptr )
			return head;

		int distance = 0;
		ListNode *target = head;
		ListNode *curNode = head;
		while ( curNode->next != nullptr )
		{
			if ( distance < n )
			{
				++distance;
			}
			else
			{
				target = target->next;
			}
			
			curNode = curNode->next;
		}

		if ( distance < n )
		{
			ListNode *toDelete = head;
			head = head->next;
			//delete toDelete;
		}
		else
		{
			ListNode *toDelete = target->next;
			target->next = toDelete->next;
			//delete toDelete;
		}

		return head;
	}
};

//int main()
//{
//	ListNode a( 1 );
//	ListNode b( 2 );
//	ListNode c( 3 );
//	ListNode d( 4 );
//	ListNode e( 5 );
//	ListNode f( 6 );
//
//	a.next = &b;
//	b.next = &c;
//	c.next = &d;
//	d.next = &e;
//	e.next = &f;
//
//	DeleteLinkNode test;
//	ListNode *curNode = test.removeNthFromEnd( &a , 1 );
//	
//	while ( curNode != nullptr )
//	{
//		cout << curNode->val << "->";
//		curNode = curNode->next;
//	}
//	cout << endl;
//
//	system( "pause" );
//	return 0;
//}