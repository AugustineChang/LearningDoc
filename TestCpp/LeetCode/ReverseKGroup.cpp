/*
给你一个链表，每 k 个节点一组进行翻转，请你返回翻转后的链表。
k 是一个正整数，它的值小于或等于链表的长度。
如果节点总数不是 k 的整数倍，那么请将最后剩余的节点保持原有顺序。

示例 :
给定这个链表：1->2->3->4->5
当 k = 2 时，应当返回: 2->1->4->3->5
当 k = 3 时，应当返回: 3->2->1->4->5

说明 :
你的算法只能使用常数的额外空间。
你不能只是单纯的改变节点内部的值，而是需要实际的进行节点交换。

来源：力扣（LeetCode）
链接：https://leetcode-cn.com/problems/reverse-nodes-in-k-group
著作权归领扣网络所有。商业转载请联系官方授权，非商业转载请注明出处。
*/

#include "Common.h"
#include <vector>
#include <iostream>
using namespace std;

struct ReverseData
{
	ListNode* Start;
	ListNode* End;
};

class ReverseKGroup
{
public:
	ListNode* reverseKGroup( ListNode* head, int k ) 
	{
		vector<ReverseData> dataList;
		dataList.reserve( 10 );

		ListNode beforeHead( 0 );
		beforeHead.next = head;

		ListNode *curNode = head;
		while( curNode != nullptr )
		{
			ReverseData newData;
			newData.Start = curNode;
			newData.End = curNode;

			dataList.push_back( newData );

			curNode = curNode->next;
		}

		int dataLen = dataList.size();
		int maxStep = 1;
		while( maxStep < k )
		{
			maxStep *= 2;
		}
		int step = 2;

		while( step <= maxStep )
		{
			ListNode *lastNode = &beforeHead;
			for( int i = 0; i < dataLen; i += k )
			{
				if( i + k - 1 >= dataLen )
					break;

				for( int j = 0; j < k; j += step )
				{
					int index1 = i + j;
					int indexInK = j + step / 2;
					int index2 = i + indexInK;
					if( indexInK < k )
					{
						swapAndMerge( lastNode, dataList[index1], dataList[index2] );
					}

					lastNode = dataList[index1].End;
				}
			}
			step *= 2;
		}

		return beforeHead.next;
	}

private:

	void swapAndMerge( ListNode *beforeStart, ReverseData &data1, ReverseData &data2 )
	{
		//swap
		ListNode *n1_start = data1.Start;
		ListNode *n2_start = data2.Start;
		ListNode *n2_end_next = data2.End->next;

		beforeStart->next = n2_start;
		data2.End->next = n1_start;
		data1.End->next = n2_end_next;

		//merge
		data1.Start = n2_start;
	}
};

//int main()
//{
//	int a[5] = { 2,5,12,67,100 };
//	int b[6] = { 1,2,3,4,5,6 };
//	int c[9] = { 1,2,3,4,5,6,7,8,9 };
//
//	ListNode *input = createList( b, 6 );
//
//	ReverseKGroup test;
//	ListNode* output = test.reverseKGroup( input, 6 );
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