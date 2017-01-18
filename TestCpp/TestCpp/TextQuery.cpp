#include "TextQuery.h"
#include "TextQueryResult.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

TextQuery::TextQuery() :isReady( false )
{
	lineList = new std::vector<std::string>();
}

TextQuery::TextQuery( const TextQuery &copy ) : isReady( true )
{
	lineList = new std::vector<std::string>();

	auto start = copy.lineList->begin();
	auto end = copy.lineList->end();
	for ( auto i = start; i != end; ++i )
	{
		lineList->push_back( *i );
	}
}

TextQuery::~TextQuery()
{
	delete lineList;

	for ( auto i = word2Line.begin(); i != word2Line.end(); ++i )
	{
		delete i->second;
	}
}


TextQuery& TextQuery::operator=( const TextQuery &copy )
{
	lineList->clear();

	auto start = copy.lineList->begin();
	auto end = copy.lineList->end();
	for ( auto i = start; i != end; ++i )
	{
		lineList->push_back( *i );
	}
	return *this;
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
		isReady = true;
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
	if ( !isReady ) return TextQueryResult( word , nullptr , nullptr );

	auto iter = word2Line.find( word );
	return TextQueryResult( word , lineList , iter == word2Line.end() ? nullptr : iter->second );
}