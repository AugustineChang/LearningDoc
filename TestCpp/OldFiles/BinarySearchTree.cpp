#include "BinarySearchTree.h"
#include <iostream>
#include <vector>


BinarySearchTree::BinarySearchTree() : treeRoot( nullptr )
{
}


BinarySearchTree::~BinarySearchTree()
{
}

void BinarySearchTree::insertNode( int newVal )
{
	if ( treeRoot == nullptr )
	{
		treeRoot = new TreeNode( newVal );
	}
	else
	{
		TreeNode *insertPoint = treeRoot;
		int insertPos = -1;
		while ( insertPos < 0 )
		{
			if ( newVal < insertPoint->value )
			{
				if ( insertPoint->leftChild == nullptr )
					insertPos = 0;
				else
					insertPoint = insertPoint->leftChild;
			}
			else
			{
				if ( insertPoint->rightChild == nullptr )
					insertPos = 1;
				else
					insertPoint = insertPoint->rightChild;
			}
		}

		TreeNode *newNode = new TreeNode( newVal );
		if ( insertPos == 0 )
			insertPoint->leftChild = newNode;
		else if ( insertPos == 1 )
			insertPoint->rightChild = newNode;
	}
}

void BinarySearchTree::insertNodes( int * const values , int length )
{
	for ( int i = 0; i < length; ++i )
	{
		insertNode( values[i] );
	}
}

void BinarySearchTree::eraseNode( int val )
{
	TreeNode *father = nullptr;
	TreeNode *target = FindNode( val , &father );

	if ( target == nullptr )//没找到目标
		return;
	
	bool hasLeft = target->leftChild != nullptr;
	bool hasRight = target->rightChild != nullptr;
	if ( hasLeft && hasRight )//有二子
	{
		//寻找先驱节点 即左支最右节点
		TreeNode *precursorFather = target;
		TreeNode *precursor = target->leftChild;

		while ( precursor->rightChild != nullptr )
		{
			precursorFather = precursor;
			precursor = precursor->rightChild;
		}

		//先驱阶段成为本节点
		target->value = precursor->value;

		//删除先驱节点
		if ( precursorFather == target )//没有进入while循环
			precursorFather->leftChild = precursor->leftChild;
		else
			precursorFather->rightChild = precursor->leftChild;
		delete precursor;
	}
	else if ( hasLeft || hasRight )//有一子
	{
		//子成为本节点
		TreeNode *child = hasLeft ? target->leftChild : target->rightChild;

		target->value = child->value;
		target->leftChild = child->leftChild;
		target->rightChild = child->rightChild;

		delete child;
	}
	else//无子
	{
		//简单删除
		if ( father == nullptr )
		{
			treeRoot = nullptr;
		}
		else
		{
			if ( father->leftChild == target )
				father->leftChild = nullptr;
			else if ( father->rightChild == target )
				father->rightChild = nullptr;
		}

		delete target;
	}
}

void BinarySearchTree::displayTree()
{
	if ( treeRoot == nullptr )
	{
		std::cout << "Empty Tree!" << std::endl;
	}
	else
	{
		/*std::vector<std::vector<int>> printList;
		preorder( treeRoot , 0 , printList );

		auto iter = printList.begin();
		auto end = printList.end();

		for ( ; iter != end; iter++ )
		{
			auto iter2 = iter->begin();
			auto end2 = iter->end();

			for ( ; iter2 != end2; iter2++ )
			{
				std::cout << *iter2 << ' ';
			}
			std::cout << std::endl;
		}*/

		inorder( treeRoot );
		std::cout << std::endl;
	}
}

TreeNode * BinarySearchTree::FindNode( int findVal )
{
	TreeNode * curNode = treeRoot;
	while ( curNode != nullptr )
	{
		if ( findVal == curNode->value )
		{
			return curNode;
		}
		else if ( findVal < curNode->value )
		{
			curNode = curNode->leftChild;
		}
		else
		{
			curNode = curNode->rightChild;
		}
	}

	return nullptr;
}

TreeNode * BinarySearchTree::FindNode( int findVal , TreeNode **parentNode )
{
	TreeNode * curNode = treeRoot;
	*parentNode = nullptr;
	while ( curNode != nullptr )
	{
		if ( findVal == curNode->value )
		{
			return curNode;
		}
		else if ( findVal < curNode->value )
		{
			*parentNode = curNode;
			curNode = curNode->leftChild;
		}
		else
		{
			*parentNode = curNode;
			curNode = curNode->rightChild;
		}
	}

	return nullptr;
}

void BinarySearchTree::inorder( TreeNode *curNode )
{
	if ( curNode == nullptr ) return;

	inorder( curNode->leftChild );

	std::cout << curNode->value << '-';

	inorder( curNode->rightChild );
}

void BinarySearchTree::preorder( TreeNode *curNode , size_t depth , std::vector<std::vector<int>> &displayList )
{
	if ( curNode == nullptr ) return;

	if ( displayList.size() <= depth )
	{
		std::vector<int> depthList;
		depthList.push_back( curNode->value );

		displayList.push_back( depthList );
	}
	else
	{
		displayList[depth].push_back( curNode->value );
	}

	preorder( curNode->leftChild , depth + 1 , displayList );
	preorder( curNode->rightChild , depth + 1 , displayList );
}