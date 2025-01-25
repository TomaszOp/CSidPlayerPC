#ifndef MOS_6510_H
#define MOS_6510_H

#include "Tools.h"
#include "RAM.h"

class MOS_6510
{
public:
	MOS_6510();
	MOS_6510(RAM & ram);
	~MOS_6510();

	void initCPU(unsigned int mempos);
	byte CPU();

	unsigned int PC, pPC, addr, storadd;
	short int A, T, SP;

	char cycles;

	RAM Ram;

private:
	static const byte flagsw[];
	static const byte branchflag[];
	byte X, Y, IR, ST;  //STATUS-flags: N V - B D I Z C
};

#endif