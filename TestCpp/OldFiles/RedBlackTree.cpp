#include "RedBlackTree.h"
#include <iostream>
#include <assert.h>

RedBlackTree::RedBlackTree() : treeRoot( nullptr )
{
}


RedBlackTree::~RedBlackTree()
{
}

bool RedBlackTree::GetValue( int key , int &value )
{
	value = 0;
	TreeNode *node = FindNode( key );
	
	if ( node == nullptr ) return false;
	else
	{
		value = node->value;
		return true;
	}
}

void RedBlackTree::insertNode( int key , int value )
{
	if ( treeRoot == nullptr )
	{
		treeRoot = new TreeNode( key , value );
	}
	else
	{
		//查找插入点
		int counter = 0;
		TreeNode *path[100];
		TreeNode *insertPoint = treeRoot;

		int insertPos = -1;
		while ( insertPos < 0 )
		{
			path[counter] = insertPoint;
			++counter;

			if ( key < insertPoint->key )
			{
				if ( insertPoint->leftChild == nullptr )
					insertPos = 0;
				else
				{
					insertPoint = insertPoint->leftChild;
				}
			}
			else
			{
				if ( insertPoint->rightChild == nullptr )
					insertPos = 1;
				else
				{
					insertPoint = insertPoint->rightChild;
				}
			}
		}

		//按标准插入 红色节点
		TreeNode *newNode = new TreeNode( key , value , true );
		if ( insertPos == 0 )
			insertPoint->leftChild = newNode;
		else if ( insertPos == 1 )
			insertPoint->rightChild = newNode;


		//按路径 逆向检测节点
		for ( int i = counter - 1; i >= 0; --i )
		{
			checkNode( path[i] );
		}
	}
}

void RedBlackTree::displayTree()
{
	if ( treeRoot == nullptr )
	{
		std::cout << "Empty Tree!" << std::endl;
	}
	else
	{
		inorder( treeRoot , 0 );
		std::cout << std::endl;
	}
}

void RedBlackTree::showSquence()
{
	if ( treeRoot == nullptr )
	{
		std::cout << "Empty Tree!" << std::endl;
	}
	else
	{
		inorder2( treeRoot );
		std::cout << std::endl;
	}
}

TreeNode * RedBlackTree::FindNode( int key )
{
	TreeNode * curNode = treeRoot;
	while ( curNode != nullptr )
	{
		if ( key == curNode->key )
		{
			return curNode;
		}
		else if ( key < curNode->key )
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

void RedBlackTree::inorder( TreeNode *curNode , int depth )
{
	if ( curNode == nullptr )
	{
		std::cout << "Nil";
		return;
	}

	std::cout << "|" << depth << "( ";

	inorder( curNode->leftChild , depth + 1 );

	std::cout << ( getIsRed( curNode->leftChild ) ? "<=" : "<-" ) << curNode->key
		<< ( getIsRed( curNode->rightChild ) ? "=>" : "->" );

	inorder( curNode->rightChild , depth + 1 );

	std::cout << " )" << depth << "|";
}

void RedBlackTree::inorder2( TreeNode *curNode )
{
	if ( curNode == nullptr )
		return;

	inorder2( curNode->leftChild );

	std::cout << curNode->key << "-";

	inorder2( curNode->rightChild );
}

void RedBlackTree::rotateLeft( TreeNode *curNode )
{
	if ( curNode == nullptr ) return;
	if ( !getIsRed( curNode->rightChild ) ) return;

	TreeNode *curRight = curNode->rightChild;

	TreeNode tempNode = *curNode;
	*curNode = *curRight;
	*curRight = tempNode;

	curRight->rightChild = curNode->leftChild;
	curNode->leftChild = curRight;

	curNode->isRed = false;
	curRight->isRed = true;
}

void RedBlackTree::rotateRight( TreeNode *curNode )
{
	if ( curNode == nullptr ) return;
	if ( !getIsRed( curNode->leftChild ) ) return;

	TreeNode *curLeft = curNode->leftChild;

	TreeNode tempNode = *curNode;
	*curNode = *curLeft;
	*curLeft = tempNode;

	curLeft->leftChild = curNode->rightChild;
	curNode->rightChild = curLeft;

	curNode->isRed = false;
	curLeft->isRed = true;
}

void RedBlackTree::flipColor( TreeNode *curNode )
{
	if ( curNode == nullptr )return;

	if ( getIsRed( curNode->leftChild ) && getIsRed( curNode->rightChild ) )
	{
		curNode->isRed = true;
		curNode->leftChild->isRed = false;
		curNode->rightChild->isRed = false;
	}
}

void RedBlackTree::checkNode( TreeNode *curNode )
{
	if ( !getIsRed( curNode->leftChild ) && getIsRed( curNode->rightChild ) ) 
		rotateLeft( curNode );

	bool isLeftLeftRed = curNode->leftChild == nullptr ? false : getIsRed( curNode->leftChild->leftChild );
	if ( getIsRed( curNode->leftChild ) && isLeftLeftRed ) 
		rotateRight( curNode );
	
	if ( getIsRed( curNode->leftChild ) && getIsRed( curNode->rightChild ) ) 
		flipColor( curNode );
}
