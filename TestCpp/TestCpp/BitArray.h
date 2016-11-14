#pragma once
class BitArray
{
public:
	BitArray();
	~BitArray();

	void setValue( int index , bool value );
	bool getValue( int index );
	int getMaxLen();
private:
	const int maxLen = 32;
	unsigned long bitArray;
};

