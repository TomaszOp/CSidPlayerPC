#include "RAM.h"

RAM::RAM(void)
{
	memory = (byte*)malloc(sizeof(byte)* MAX_DATA_LEN);
}

RAM::RAM(byte* _memory)
{
	memory = _memory;
}

RAM::~RAM()
{
	free(memory);
}