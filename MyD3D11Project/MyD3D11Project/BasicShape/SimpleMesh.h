#pragma once
#include "BasicShape.h"
class SimpleMesh : public BasicShape
{
public:
	SimpleMesh();
	~SimpleMesh();

	virtual void createObjectMesh() override;

private:

	std::string filePath;
};

