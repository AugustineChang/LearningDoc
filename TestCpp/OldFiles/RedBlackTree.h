#pragma once
struct TreeNode
{
	TreeNode( int k , int v , bool red = false ) : key( k ) , value( v ) , isRed( red ) ,
		leftChild( nullptr ) , rightChild( nullptr )
	{
	}

	int key;
	int value;

	bool isRed;
	TreeNode *leftChild;
	TreeNode *rightChild;
};

class RedBlackTree
{
public:
	RedBlackTree();
	~RedBlackTree();

	bool GetValue( int key , int &value );
	void insertNode( int key , int value );

	void displayTree();
	void showSquence();

private:

	TreeNode * FindNode( int key );
	void inorder( TreeNode *curNode , int depth );
	void inorder2( TreeNode *curNode );
	bool getIsRed( TreeNode *curNode )
	{
		return curNode == nullptr ? false : curNode->isRed;
	}

	void rotateLeft( TreeNode *curNode );
	void rotateRight( TreeNode *curNode );
	void flipColor( TreeNode *curNode );

	void checkNode( TreeNode *curNode );

	TreeNode * treeRoot;
};

