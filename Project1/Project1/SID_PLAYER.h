#ifndef SID_PLAYER_H
#define SID_PLAYER_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <SDL_config.h>
#include <SDL.h>
#include <SDL_audio.h>

#include "SID_FILE.h"
#include "MOS_6510.h"
#include "MOS_SID.h"



struct CallbackData
{
public:
	int subtune;
	float framecnt;
	float frame_sampleperiod;
	
	float CPUtime;
	char finished;
	char dynCIA;

	unsigned int initaddr;
	unsigned int playaddr;
	unsigned int playaddf;
	unsigned int SID_address[3];

	int SIDamount;
	int requested_SID_model;
	int sampleratio;

	byte timermode[0x20];

	CallbackData() 
	{
		SID_address[0] = 0xD400;
		SID_address[1] = 0;
		SID_address[2] = 0;

		SIDamount = 1;
		requested_SID_model = -1; 
	};
	

	CallbackData(MOS_SID & ms): MosSid(ms)
	{
		SID_address[0] = 0xD400;
		SID_address[1] = 0;
		SID_address[2] = 0;

		SIDamount = 1;
		requested_SID_model = -1;
	};

	MOS_SID MosSid;
};


class SidPlayer
{
	public:
		SidPlayer();
		SidPlayer(SidFile &sf, CallbackData &cd);

		int LoadFile(char * filePath);
		int InserFile(char* fileBuffer, unsigned int sidLength);
		int InitSDL();
		int Play();
		int Stop();

		static void play(void* voidCallData, Uint8 *stream, int len);

		RAM Ram;

		MOS_6510 Mos6510{ Ram };

		SidFile sidFile{ Ram };

		MOS_SID MosSid{ Mos6510 };

		CallbackData callbackData{ MosSid };



	private:
		SDL_AudioSpec soundspec;
		void init(int subt);

		int tunelength;
		int default_tunelength;
		int minutes;
		int seconds;

		
};




#endif