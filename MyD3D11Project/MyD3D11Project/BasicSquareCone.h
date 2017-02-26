#pragma once
#include "BasicShape.h"


class BasicSquareCone : public BasicShape
{
public:
	BasicSquareCone();
	~BasicSquareCone();

protected:

	virtual void createObjectMesh() override;
};

