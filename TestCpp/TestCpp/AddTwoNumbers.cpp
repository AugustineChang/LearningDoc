/*You are given two linked lists representing two non - negative numbers.The digits are stored in reverse order and each of their nodes contain a single digit.Add the two numbers and return it as a linked list.

Input: ( 2 -> 4 -> 3 ) + ( 5 -> 6 -> 4 )
Output : 7 -> 0 -> 8*/

#include<stdio.h>
#include<vector>

using namespace std;


// Definition for singly-linked list.
struct ListNode
{
	int val;
	ListNode *next;
	ListNode( int x ) : val( x ) , next( nullptr )
	{
	}
};

class Solution
{
public:
	ListNode *addTwoNumbers( ListNode* l1 , ListNode* l2 )
	{
		ListNode* result = new ListNode( 0 );
		
		handleAddOper( result , l1 , l2 , 0 );

		return result;
	}


	void handleAddOper( ListNode*result , ListNode* l1 , ListNode* l2, int overNum )
	{
		result->val = l1->val + l2->val + overNum;
		int curOver = result->val / 10;
		result->val = result->val % 10;

		int count = 0;
		
		if( l1->next == nullptr )
		{
			l1->next = new ListNode( 0 );
			count++;
		}

		if( l2->next == nullptr )
		{
			l2->next = new ListNode( 0 );
			count++;
		}

		if( curOver == 0 )
		{
			count++;
		}

		if( count != 3 )
		{
			result->next = new ListNode( 0 );
			handleAddOper( result->next , l1->next , l2->next , curOver );
		}
	}
};


void printLinkList( ListNode* list )
{
	ListNode* curNode = list;
	while( true )
	{
		printf( "%d" , curNode->val );

		if( curNode->next == nullptr )
		{
			printf( "\n" );
			break;
		}
		else
		{
			printf( " -> " );
			curNode = curNode->next;
		}
	}
}


void generateLinkList( ListNode*& list , const vector<int>& vector )
{
	size_t len = vector.size();
	list = new ListNode( vector[0] );
	ListNode* curNode = list;

	for( size_t index = 1; index < len; index++ )
	{
		curNode->next = new ListNode( vector[index] );
		curNode = curNode->next;
	}
}


//int main()
//{
//	Solution test;
//
//	vector<int> l1Vec = { 9 , 9 , 9 , 9 };
//	vector<int> l2Vec = { 8 , 9 , 9 , 9 };
//	ListNode* l1 = nullptr;
//	ListNode* l2 = nullptr;
//	generateLinkList( l1 , l1Vec );
//	generateLinkList( l2 , l2Vec );
//	printLinkList( l1 );
//	printLinkList( l2 );
//
//	ListNode* result = test.addTwoNumbers( l1 , l2 );
//	printLinkList( result );
//
//	return 0;
//}