#ifndef CSID_H
#define CSID_H

#include "SID_PLAYER.h"

class  CSid
{
public:

	CSid();
	~CSid();

	int Init();
	int LoadFile(char * filePath);
	int InserFile(char * fileBuffer, unsigned int sidLength);
	int Play();
	int Stop();

	SidPlayer sidPlayer;

private:

	
};


#endif