#pragma once
#include <vector>

struct TreeNode
{
	TreeNode( int val ) 
		:value( val ) , leftChild( nullptr ) , rightChild( nullptr )
	{
	}

	int value;
	TreeNode *leftChild;
	TreeNode *rightChild;
};

class BinarySearchTree
{
public:
	BinarySearchTree();
	~BinarySearchTree();

	void insertNode( int newVal );
	void insertNodes( int * const values , int length );

	void eraseNode( int val );

	void displayTree();

private:

	TreeNode *FindNode( int findVal );
	TreeNode *FindNode( int findVal , TreeNode **parentNode );

	void inorder( TreeNode *curNode );
	void preorder( TreeNode *curNode , size_t depth , std::vector<std::vector<int>> &displayList );

	TreeNode *treeRoot;
};

