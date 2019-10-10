#include "BitArray.h"


BitArray::BitArray()
{
	bitArray = 0;
}


BitArray::~BitArray()
{
}

void BitArray::setValue( int index , bool value )
{
	if ( index < 0 || index >= maxLen )return;

	if ( value ) bitArray |= 1UL << index;
	else bitArray &= ~( 1UL << index );
}

bool BitArray::getValue( int index )
{
	if ( index < 0 || index >= maxLen )return false;

	return bitArray & ( 1UL << index );
}

int BitArray::getMaxLen()
{
	return maxLen;
}
