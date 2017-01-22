#include "Folder.h"
#include "Message.h"


Folder::Folder()
{
}

Folder::Folder( const Folder& copy )
{
	for ( Message* msg : copy.messageSet )
	{
		messageSet.insert( msg );
		msg->saveToFolder( this );
	}
}

Folder& Folder::operator=( const Folder &copy )
{
	//copy to temp
	std::set<Message*> tempSet;
	for ( Message* msg : copy.messageSet )
	{
		tempSet.insert( msg );
	}

	//delete current
	for ( Message* msg : messageSet )
	{
		msg->removeFromFolder( this );
	}
	messageSet.clear();

	//copy to current from temp
	for ( Message* msg : tempSet )
	{
		msg->saveToFolder( this );
		messageSet.insert( msg );
	}

	return *this;
}

Folder::~Folder()
{
	for ( Message* msg : messageSet )
	{
		msg->removeFromFolder( this );
	}
	messageSet.clear();
}

void Folder::AddToFolder( Message* msg )
{
	messageSet.insert( msg );
	msg->saveToFolder( this );
}

void Folder::RemoveFromFolder( Message* msg )
{
	messageSet.erase( msg );
}


void Folder::ShowAllMessage()
{
	for ( Message *msg : messageSet )
	{
		msg->ShowMessage();
	}
}