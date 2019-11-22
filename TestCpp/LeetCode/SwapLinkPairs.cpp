/*
给定一个链表，两两交换其中相邻的节点，并返回交换后的链表。
你不能只是单纯的改变节点内部的值，而是需要实际的进行节点交换。

示例:
给定 1->2->3->4, 你应该返回 2->1->4->3.

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/swap-nodes-in-pairs
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include <iostream>
#include "Common.h"
using namespace std;

class SwapLinkPairs 
{
public:
	ListNode* swapPairs( ListNode* head ) 
	{
		if( head == nullptr )
			return nullptr;
		else if( head->next == nullptr )
			return head;

		ListNode* newHead = head->next;
		ListNode* lastNode = nullptr;
		ListNode* curNode = head;
		while( curNode != nullptr && curNode->next != nullptr )
		{
			ListNode* nextNode = curNode->next;
			ListNode* nextNextNode = nextNode->next;

			if( lastNode != nullptr )
			{
				lastNode->next = nextNode;
			}
			nextNode->next = curNode;
			curNode->next = nextNextNode;

			lastNode = curNode;
			curNode = nextNextNode;
		}

		return newHead;
	}
};

int main()
{
	int a[5] = { 2,5,12,67,100 };
	int b[3] = { 8,23,24 };
	int c[7] = { 1,6,10,11,15,17,50 };

	ListNode *input = createList( c, 7 );

	SwapLinkPairs test;
	ListNode* output = test.swapPairs( input );

	while( output != nullptr )
	{
		cout << output->val << "->";
		output = output->next;
	}
	cout << endl;

	system( "pause" );
	return 0;
}