#pragma once
#include<set>
class Message;

class Folder
{
public:
	Folder();

	Folder( const Folder& );
	Folder& operator=( const Folder& );
	~Folder();

	void AddToFolder( Message* );
	void RemoveFromFolder( Message* );
	void ShowAllMessage();

private:
	std::set<Message*> messageSet;
};

