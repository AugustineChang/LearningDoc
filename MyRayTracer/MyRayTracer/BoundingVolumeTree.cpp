#include "BoundingVolumeTree.h"
#include "MyMath.h"

BoundingVolumeTree::BoundingVolumeTree()
	: leftTree( nullptr ) , rightTree( nullptr ) , cachedBox()
{
}

BoundingVolumeTree::BoundingVolumeTree( Hitable ** objList , int numOfList , float exposureTime )
	: leftTree( nullptr ) , rightTree( nullptr ) , cachedBox()
{
	if ( numOfList <= 0 )
		return;

	//1.random Axis and sort primitives along the axis
	float randAxis = MyMath::getRandom01() * 3.0f;
	if ( randAxis <= 1.0f )
	{
		sort( objList , numOfList , &BoundingVolumeTree::compareAxisX );
	}
	else if ( randAxis <= 2.0f )
	{
		sort( objList , numOfList , &BoundingVolumeTree::compareAxisY );
	}
	else
	{
		sort( objList , numOfList , &BoundingVolumeTree::compareAxisZ );
	}

	//2.put half in each subtree
	if ( numOfList == 1 )
	{
		leftTree = objList[0];
		leftTree->getBoundingBox( exposureTime , cachedBox );
	}
	else
	{
		int mid = ( numOfList - 1 ) / 2;
		int numOfLeft = mid + 1;
		leftTree = new BoundingVolumeTree( objList , numOfLeft , exposureTime );
		rightTree = new BoundingVolumeTree( objList + numOfLeft , numOfList - numOfLeft , exposureTime );

		BoundingBox leftBox , rightBox;
		leftTree->getBoundingBox( exposureTime , leftBox );
		rightTree->getBoundingBox( exposureTime , rightBox );

		cachedBox = BoundingBox( leftBox , rightBox );
	}
}

bool BoundingVolumeTree::hitTest( const Ray &ray , float t_min , float t_max , HitResult& hitResult )
{
	HitResult leftResult , rightResult;
	if ( !cachedBox.hitTest( ray , t_min , t_max , leftResult ) )
		return false;

	bool isLeftHit = false;
	if ( leftTree != nullptr )
	{
		isLeftHit = leftTree->hitTest( ray , t_min , t_max , leftResult );
	}

	bool isRightHit = false;
	if ( rightTree != nullptr )
	{
		isRightHit = rightTree->hitTest( ray , t_min , t_max , rightResult );
	}

	if ( isLeftHit && isRightHit )
	{
		if ( leftResult.t < rightResult.t )
			hitResult = leftResult;
		else
			hitResult = rightResult;
	}
	else if ( isLeftHit )
	{
		hitResult = leftResult;
	}
	else if ( isRightHit )
	{
		hitResult = rightResult;
	}
	else
		return false;

	return true;
} 

bool BoundingVolumeTree::getBoundingBox( float exposureTime , BoundingBox &aabb )
{
	if ( cachedBox.isEmpty() )
		return false;
	else
	{
		aabb = cachedBox;
		return true;
	}
}

void BoundingVolumeTree::sort( Hitable **objList , int numOfList , CompareFunc compFunc )
{
	Hitable **tempList = new Hitable *[numOfList];

	mergeSort( objList , 0 , numOfList - 1 , tempList , compFunc );

	delete[] tempList;
}

void BoundingVolumeTree::merge( Hitable **objList , int start , int mid , int end ,
	Hitable **tempList , CompareFunc compFunc )
{
	int headA = start;
	int headB = mid + 1;
	int headRet = 0;

	while ( headA <= mid && headB <= end )
	{
		bool isAFirst = false;
		if ( compFunc != nullptr )
			isAFirst = ( this->*compFunc )( objList[headA] , objList[headB] );

		if ( isAFirst )
		{
			tempList[headRet] = objList[headA];
			++headA;
		}
		else
		{
			tempList[headRet] = objList[headB];
			++headB;
		}
		++headRet;
	}

	while ( headA <= mid )
	{
		tempList[headRet] = objList[headA];
		++headA;
		++headRet;
	}

	while ( headB <= end )
	{
		tempList[headRet] = objList[headB];
		++headB;
		++headRet;
	}

	for ( int i = 0; i < headRet; ++i )
	{
		objList[start + i] = tempList[i];
	}
}

void BoundingVolumeTree::mergeSort( Hitable **objList , int start , int end ,
	Hitable **tempList, CompareFunc compFunc )
{
	if ( start >= end )
		return;

	int mid = start + ( end - start ) / 2;
	mergeSort( objList , start , mid , tempList , compFunc );
	mergeSort( objList , mid + 1 , end , tempList , compFunc );

	merge( objList , start , mid , end , tempList , compFunc );
}

bool BoundingVolumeTree::compareAxisX( Hitable *objA , Hitable *objB )
{
	return compareAxis( objA , objB , 0 );
}

bool BoundingVolumeTree::compareAxisY( Hitable *objA , Hitable *objB )
{
	return compareAxis( objA , objB , 1 );
}

bool BoundingVolumeTree::compareAxisZ( Hitable *objA , Hitable *objB )
{
	return compareAxis( objA , objB , 2 );
}

bool BoundingVolumeTree::compareAxis( Hitable *objA , Hitable *objB , int axis )
{
	BoundingBox boxA , boxB;

	bool validA = objA->getBoundingBox( 0.0f , boxA );
	bool validB = objB->getBoundingBox( 0.0f , boxB );

	if ( !validA && validB )
		return false;

	if ( validA && !validB )
		return true;

	if ( !validA && !validB )
		return true;

	float posA = boxA.GetCenter()[axis];
	float posB = boxB.GetCenter()[axis];
	return posA < posB;
}
