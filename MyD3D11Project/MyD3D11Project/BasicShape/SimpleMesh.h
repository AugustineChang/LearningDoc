#pragma once
#include "BasicShape.h"
class SimpleMesh : public BasicShape
{
public:
	SimpleMesh();
	~SimpleMesh();

private:

	virtual void createObjectMesh() override;

	std::string filePath;
};

