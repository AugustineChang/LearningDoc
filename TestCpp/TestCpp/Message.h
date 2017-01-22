#pragma once
#include <set>

class Folder;

class Message
{
public:
	Message( const std::string str = "" );

	Message( const Message & );
	Message& operator=( const Message & );
	~Message();

	void ShowMessage();
	void SetMessage( const std::string& );

	void saveToFolder( Folder* );
	void removeFromFolder( Folder* );
	void showReferenceCount();

private:
	std::string content;
	std::set<Folder *> folderSet;
};

