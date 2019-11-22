#include "Common.h"

ListNode* ::createList( int* arr, int len )
{
	ListNode* head = nullptr;
	ListNode* curNode = nullptr;
	for( int i = 0; i < len; ++i )
	{
		ListNode* newNode = new ListNode( arr[i] );
		if( head == nullptr )
		{
			head = newNode;
			curNode = head;
		}
		else
		{
			curNode->next = newNode;
			curNode = curNode->next;
		}
	}

	return head;
}