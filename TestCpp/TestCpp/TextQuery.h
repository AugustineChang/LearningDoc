#pragma once
#include <vector>
#include <map>
#include <set>

class TextQueryResult;

class TextQuery
{
public:
	TextQuery();
	TextQuery( const TextQuery & );
	~TextQuery();

	TextQuery& operator=( const TextQuery & );

	void readFromStream();
	TextQueryResult getQueryResult( std::string word );

private:
	void findWordInLine( const std::string &line , int lineIndex );

	bool isReady;
	std::vector<std::string> *lineList;
	std::map<std::string , std::set<int>*> word2Line;
};

