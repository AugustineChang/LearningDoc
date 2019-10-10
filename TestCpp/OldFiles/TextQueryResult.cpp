#include "TextQueryResult.h"
#include <iostream>


TextQueryResult::TextQueryResult( std::string word , std::vector<std::string> *text , std::set<int> *set )
	:quertWord( word ) , totalText( text ) , lineIndexSet( set )
{
}


TextQueryResult::~TextQueryResult()
{
}


void TextQueryResult::showQueryResult()
{
	std::cout << "Query Word:" << quertWord.c_str() << std::endl;

	if ( lineIndexSet == nullptr )
	{
		std::cout << "\tNo Result!!" << std::endl;
	}
	else
	{
		auto start = lineIndexSet->begin();
		auto end = lineIndexSet->end();
		for ( auto i = start; i != end; ++i )
		{
			std::cout << "\tline" << *i << ":"
				<< ( *totalText )[*i].c_str() << std::endl;
		}
	}
}