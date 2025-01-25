#ifndef RAM_H
#define RAM_H

#include <stdlib.h>
#include "Tools.h"

class  RAM
{
public:
	RAM();
	RAM(byte * _memory);

	byte * memory;

private:
};

#endif