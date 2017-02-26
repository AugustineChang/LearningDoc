#pragma once
#include "BasicShape.h"


class BasicSquareCone : public BasicShape
{
public:
	BasicSquareCone();
	~BasicSquareCone();

	virtual const std::vector<CustomVertex>& getVertices() const override;
	virtual const std::vector<unsigned int>& getIndices() const override;
};

