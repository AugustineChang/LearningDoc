#include "SimpleMath.h"
#include <math.h>

int SimpleMath::floorToInt( float val )
{
	return static_cast<int>( floorf( val ) );
}
