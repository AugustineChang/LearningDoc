#include "TextQuery.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

TextQuery::TextQuery()
{
	lineList = new std::vector<std::string>();
}


TextQuery::~TextQuery()
{
	delete lineList;

	for ( auto i = word2Line.begin(); i != word2Line.end(); ++i )
	{
		delete i->second;
	}
}


void TextQuery::readFromStream()
{
	std::ifstream input( "../Debug/input.txt" );
	if ( input )
	{
		std::string line;
		while ( std::getline( input , line ) )
		{
			findWordInLine( line , lineList->size() );
			lineList->push_back( line );
		}
	}
	
	input.close();
}

void TextQuery::findWordInLine( const std::string &line , int lineIndex )
{
	std::istringstream lineStream( line );
	std::string word;

	while ( lineStream >> word )
	{
		auto iter = word2Line.find( word );
		if ( iter == word2Line.end() )
		{
			std::set<int> *indexSet = new std::set<int>{ lineIndex };
			word2Line.insert( std::make_pair( word , indexSet ) );
		}
		else
		{
			iter->second->insert( lineIndex );
		}
	}
}

TextQueryResult TextQuery::getQueryResult( std::string word )
{
	auto iter = word2Line.find( word );
	return TextQueryResult( word , lineList , iter == word2Line.end() ? nullptr : iter->second );
}