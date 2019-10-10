#include "Message.h"
#include "Folder.h"
#include <iostream>

Message::Message( const std::string str ) : content( str )
{
}

Message::Message( const Message &copy )
{
	content = copy.content;

	for ( Folder* folder : copy.folderSet )
	{
		folderSet.insert( folder );
		folder->AddToFolder( this );
	}
}

Message& Message::operator=( const Message &copy )
{
	//copy to temp
	std::string tempStr = copy.content;
	std::set<Folder*> tempSet;
	for ( Folder* folder : copy.folderSet )
	{
		tempSet.insert( folder );
	}

	//delete current
	for ( Folder* folder : folderSet )
	{
		folder->RemoveFromFolder( this );
	}
	folderSet.clear();

	//copy to current from temp
	content = tempStr;

	for ( Folder* folder : tempSet )
	{
		folderSet.insert( folder );
		folder->AddToFolder( this );
	}

	return *this;
}

Message::~Message()
{
	for ( Folder* folder : folderSet )
	{
		folder->RemoveFromFolder( this );
	}
	folderSet.clear();
}


void Message::ShowMessage()
{
	std::cout << content.c_str() << std::endl;
}

void Message::SetMessage( const std::string& str )
{
	content = str;
}

void Message::saveToFolder( Folder *folder )
{
	folderSet.insert( folder );
}

void Message::removeFromFolder( Folder* folder )
{
	folderSet.erase( folder );
}

void Message::showReferenceCount()
{
	std::cout << content.c_str() << " in " << folderSet.size() << " folders" << std::endl;
}