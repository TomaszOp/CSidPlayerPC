#ifndef SID_FILE_H
#define SID_FILE_H

#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "Tools.h"
#include "RAM.h"

class  SidFile
{
public:
	SidFile();
	SidFile(RAM & ram);
	~SidFile();

	int LoadFile(char * filePath);
	int InserFile(char* fileBuffer, unsigned int sidLength);
	void SetToMemory(byte memory[]);
	void SetToMemory();

	int subtune;
	//int tunelength;

	unsigned int initaddr;
	unsigned int playaddr;
	unsigned int playaddf;
	unsigned int SID_address[3];
	int SIDamount;
	int requested_SID_model;
	int SID_model[3];

	RAM Ram;

private:
	char isExt(char *filename, const char* ext);
	void init();

	//from LoadFile
	// byte filedata[MAX_DATA_LEN];
	byte * filedata;

	unsigned int offs, loadaddr;
	unsigned int datalen;
	//
};



#endif