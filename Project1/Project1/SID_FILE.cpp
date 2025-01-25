#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "SID_FILE.h"

SidFile::SidFile()
{
	init();
}

SidFile::SidFile(RAM & ram) : Ram(ram)
{
	init();
}

SidFile::~SidFile()
{
	free(filedata);
}

void SidFile::init()
{
	filedata = (byte*)malloc(sizeof(byte) * MAX_DATA_LEN);
}

void SidFile::SetToMemory(byte memory[])
{
	unsigned int i;
	
	for (i = 0; i<MAX_DATA_LEN; i++)
		memory[i] = 0;

	for (i = offs + 2; i<datalen; i++)
	{
		if (loadaddr + i - (offs + 2)<MAX_DATA_LEN)
			memory[loadaddr + i - (offs + 2)] = filedata[i];
	}
}

void SidFile::SetToMemory()
{
	unsigned int i;

	for (i = 0; i<MAX_DATA_LEN; i++)
		Ram.memory[i] = 0;

	for (i = offs + 2; i<datalen; i++)
	{
		if (loadaddr + i - (offs + 2)<MAX_DATA_LEN)
			Ram.memory[loadaddr + i - (offs + 2)] = filedata[i];
	}

	Ram.memory[0] = 11;
}

int SidFile::LoadFile(char * filePath)
{
	FILE *InputFile = NULL;
	char filename[MAX_FILENAME_LEN] = "";
	int readata;

	if (isExt(filePath, ".sid") || isExt(filePath, ".SID"))
	{
		strcpy(filename, filePath);
	}
	else
	{
		#ifdef Print2Console
		//fprintf(stderr, "Only SID file:\n");
		sprintf(bufferMessage, "Only SID file:\n");
		print2Console(bufferMessage);
		#endif

		return (1);
	}

	InputFile = fopen(filename, "rb");
	if (InputFile == NULL)
	{
		#ifdef Print2Console
		//printf("SID file %s not found.\n", filename);
		sprintf(bufferMessage, "SID file %s not found.\n", filename);
		print2Console(bufferMessage);
		#endif

		return(3);
	}

	datalen = 0;

	do
	{
		readata = fgetc(InputFile);
		filedata[datalen++] = readata;
	} while (readata != EOF && datalen<MAX_DATA_LEN);


	return InserFile((char*)filedata, datalen);
}

int SidFile::InserFile(char* fileBuffer, unsigned int sidLength)
{
	char filename[MAX_FILENAME_LEN] = "";
	int readata;
	unsigned int i;
	byte timermode[0x20], SIDtitle[0x20], SIDauthor[0x20], SIDinfo[0x20];
	int strend;
	int subtune_amount;
	int preferred_SID_model[3] = { 8580.0,8580.0,8580.0 };

	datalen = sidLength;

	for (int i = 0; i < datalen; i++)
	{
		filedata[i] = (byte)fileBuffer[i];
	}


	subtune--;
	if (subtune < 0 || subtune>63)
		subtune = 0;


	#ifdef Print2Console
	//printf("%d bytes read (%s subtune %d)", --datalen, filename, subtune + 1); 
	sprintf(bufferMessage, "\n%d bytes read (%s subtune %d)", --datalen, filename, subtune + 1);
	print2Console(bufferMessage);
	#endif

	offs = filedata[7];
	loadaddr = filedata[8] + filedata[9] ? filedata[8] * 256 + filedata[9] : filedata[offs] + filedata[offs + 1] * 256;
	
	#ifdef Print2Console
	//printf("\nOffset: $%4.4X, Loadaddress: $%4.4X, Size: $%4.4X", offs, loadaddr, datalen - offs);
	sprintf(bufferMessage, "\nOffset: $%4.4X, Loadaddress: $%4.4X, Size: $%4.4X", offs, loadaddr, datalen - offs);

	//printf("\nTimermodes: ");
	sprintf(bufferMessage, "\nTimermodes: ");

	for (i = 0; i < 32; i++)
	{
		timermode[31 - i] = (filedata[0x12 + (i >> 3)] & (byte)pow(2, static_cast<double>(7 - i % 8))) ? 1 : 0;
		//printf(" %1d", timermode[31 - i]);
		sprintf(bufferMessage, " %1d", timermode[31 - i]);
	}
	#endif

	strend = 1;
	for (i = 0; i < 32; i++)
	{
		if (strend != 0)
			strend = SIDtitle[i] = filedata[0x16 + i];
		else strend = SIDtitle[i] = 0;
	}

	#ifdef Print2Console
	//printf("\nTitle: %s    ", SIDtitle);
	sprintf(bufferMessage, "\nTitle: %s    ", SIDtitle);
	print2Console(bufferMessage);
	#endif


	strend = 1;
	for (i = 0; i < 32; i++)
	{
		if (strend != 0)
			strend = SIDauthor[i] = filedata[0x36 + i];
		else strend = SIDauthor[i] = 0;
	}  
	
	
	#ifdef Print2Console
	//printf("Author: %s    ", SIDauthor);
	sprintf(bufferMessage, "\nAuthor: %s    ", SIDauthor);
	print2Console(bufferMessage);
	#endif

	strend = 1;
	for (i = 0; i < 32; i++)
	{
		if (strend != 0)
			strend = SIDinfo[i] = filedata[0x56 + i]; 
		else strend = SIDinfo[i] = 0;
	} 
	
	#ifdef Print2Console
	//printf("Info: %s", SIDinfo);
	sprintf(bufferMessage, "\nInfo: %s", SIDinfo);
	print2Console(bufferMessage);
	#endif

	initaddr = filedata[0xA] + filedata[0xB] ? filedata[0xA] * 256 + filedata[0xB] : loadaddr; playaddr = playaddf = filedata[0xC] * 256 + filedata[0xD];
	
	#ifdef Print2Console	
	//printf("\nInit:$%4.4X,Play:$%4.4X, ", initaddr, playaddr);
	sprintf(bufferMessage, "\nInit:$%4.4X,Play:$%4.4X, ", initaddr, playaddr);
	print2Console(bufferMessage);
	#endif

	subtune_amount = filedata[0xF];
	preferred_SID_model[0] = (filedata[0x77] & 0x30) >= 0x20 ? 8580 : 6581;

	#ifdef Print2Console
	//printf("Subtunes:%d , preferred SID-model:%d", subtune_amount, preferred_SID_model[0]);
	sprintf(bufferMessage, "\nSubtunes:%d , preferred SID-model:%d", subtune_amount, preferred_SID_model[0]);
	print2Console(bufferMessage);
	#endif

	preferred_SID_model[1] = (filedata[0x77] & 0xC0) >= 0x80 ? 8580 : 6581;
	preferred_SID_model[2] = (filedata[0x76] & 3) >= 2 ? 8580 : 6581;
	SID_address[1] = filedata[0x7A] >= 0x42 && (filedata[0x7A] < 0x80 || filedata[0x7A] >= 0xE0) ? 0xD000 + filedata[0x7A] * 16 : 0;
	SID_address[2] = filedata[0x7B] >= 0x42 && (filedata[0x7B] < 0x80 || filedata[0x7B] >= 0xE0) ? 0xD000 + filedata[0x7B] * 16 : 0;
	SIDamount = 1 + (SID_address[1] > 0) + (SID_address[2] > 0); 
	
	if (SIDamount >= 2)
	{
		#ifdef Print2Console
		sprintf(bufferMessage, "(SID1), %d(SID2:%4.4X)", preferred_SID_model[1], SID_address[1]);
		print2Console(bufferMessage);
		#endif
	}


	if (SIDamount == 3)
	{
		#ifdef Print2Console
		//printf(", %d(SID3:%4.4X)", preferred_SID_model[2], SID_address[2]);
		sprintf(bufferMessage, ", %d(SID3:%4.4X)", preferred_SID_model[2], SID_address[2]);
		print2Console(bufferMessage);
		#endif
	}

	if (requested_SID_model != -1)
	{
		#ifdef Print2Console
		//printf(" (requested:%d)", requested_SID_model); printf("\n");
		sprintf(bufferMessage, " (requested:%d)", requested_SID_model); 
		printf("\n");
		print2Console(bufferMessage);
		#endif
	}

	for (i = 0; i < SIDamount; i++)
	{
		if (requested_SID_model == 8580 || requested_SID_model == 6581)
			SID_model[i] = requested_SID_model;
		else
			SID_model[i] = preferred_SID_model[i];
	}

	return 0;
}


char SidFile::isExt(char *filename, const char* ext)
{ 
	char* LastDotPos = strrchr(filename, '.'); if (LastDotPos == NULL) return 0;
	if (!strcmp(LastDotPos, ext)) return 1; else return 0;
}
