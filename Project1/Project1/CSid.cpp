// CSid.cpp : Defines the entry point for the console application.
//

#include <iostream>


#include "CSid.h"


CSid::CSid()
{


}

int CSid::Init()
{
	//sidPlayer.callbackData.MosSid = &MosSid;

	return 0;
}

int CSid::LoadFile(char * filePath)
{
	return sidPlayer.LoadFile(filePath);
}

int CSid::InserFile(char* fileBuffer, unsigned int sidLength)
{
	return sidPlayer.InserFile(fileBuffer, sidLength);
}

int CSid::InitSDL()
{
	return sidPlayer.InitSDL();
}

int CSid::Play()
{
	return sidPlayer.Play();
}

int CSid::Stop()
{
	return sidPlayer.Stop();
}






















