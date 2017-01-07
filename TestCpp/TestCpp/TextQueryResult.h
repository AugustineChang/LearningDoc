#pragma once
#include <vector>
#include <set>

class TextQueryResult
{
public:
	TextQueryResult( std::string word , std::vector<std::string> *text , std::set<int> *set );
	~TextQueryResult();

	void showQueryResult();

private:
	std::string quertWord;
	std::vector<std::string> *totalText;
	std::set<int> *lineIndexSet;
};

