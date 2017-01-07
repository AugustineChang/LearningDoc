#pragma once
#include "TextQueryResult.h"
#include <vector>
#include <map>
#include <set>

class TextQuery
{
public:
	TextQuery();
	~TextQuery();

	void readFromStream();
	TextQueryResult getQueryResult( std::string word );

private:
	void findWordInLine( const std::string &line , int lineIndex );

	std::vector<std::string> *lineList;
	std::map<std::string , std::set<int>*> word2Line;
};

