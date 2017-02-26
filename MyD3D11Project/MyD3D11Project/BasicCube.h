#pragma once
#include "BasicShape.h"


class BasicCube :public BasicShape
{
public:
	BasicCube();
	~BasicCube();
	
	virtual const std::vector<CustomVertex>& getVertices() const override;
	virtual const std::vector<unsigned int>& getIndices() const override;
};

