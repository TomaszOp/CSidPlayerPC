#include "SID_PLAYER.h"
#include "stdlib.h"

SidPlayer::SidPlayer(void)
{
	callbackData.subtune = 0;

	callbackData.framecnt = 0;
	callbackData.frame_sampleperiod = DEFAULT_SAMPLERATE / PAL_FRAMERATE;

	callbackData.CPUtime = 0.0;
	callbackData.finished = 0;
	callbackData.dynCIA = 0;

	tunelength = -1;
	default_tunelength = 300;
	minutes = -1;
	seconds = -1;

	callbackData.SID_address[0] = 0xD400;
	callbackData.SID_address[1] = 0;
	callbackData.SID_address[2] = 0;

	callbackData.SIDamount = 1;
	callbackData.requested_SID_model = -1;
}

SidPlayer::SidPlayer(SidFile &sf, CallbackData &cd) : sidFile(sf), callbackData(cd)
{
	callbackData.subtune = 0;

	callbackData.framecnt = 0;
	callbackData.frame_sampleperiod = DEFAULT_SAMPLERATE / PAL_FRAMERATE;

	callbackData.CPUtime = 0.0;
	callbackData.finished = 0;
	callbackData.dynCIA = 0;

	tunelength = -1;
	default_tunelength = 300;
	minutes = -1;
	seconds = -1;

	callbackData.SID_address[0] = 0xD400;
	callbackData.SID_address[1] = 0;
	callbackData.SID_address[2] = 0;

	callbackData.SIDamount = 1;
	callbackData.requested_SID_model = -1;
}

SidPlayer::~SidPlayer()
{

}

int SidPlayer::InitSDL()
{
	callbackData.MosSid.samplerate = DEFAULT_SAMPLERATE;
	callbackData.sampleratio = callbackData.MosSid.mos_round(C64_PAL_CPUCLK / callbackData.MosSid.samplerate);
	
	if (sdlHelper.SDL__Init(SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return(1);
	}

	sdlHelper.soundspec.freq = callbackData.MosSid.samplerate;
	sdlHelper.soundspec.channels = 1;
	sdlHelper.soundspec.format = AUDIO_S16;
	sdlHelper.soundspec.samples = 16384;
	//sdlHelper.soundspec.userdata = NULL;
	//sdlHelper.soundspec.callback = play;


	sdlHelper.soundspec.userdata = &callbackData;//&callbackObject;
	sdlHelper.soundspec.callback = play; // CallbackObject::forwardCallback;

	if (sdlHelper.SDL__OpenAudio(&sdlHelper.soundspec, NULL) < 0)
	{
		fprintf(stderr, "Couldn't open audio: %s\n", sdlHelper.SDL__GetError());
		return(2);
	}

	return 0;
}

int SidPlayer::LoadFile(char * filePath)
{ 
	sidFile.LoadFile(filePath);
	sidFile.SetToMemory(callbackData.MosSid.Mos6510.Ram.memory);

	//byte e = Mos6510.Ram.memory[0];

	callbackData.subtune = sidFile.subtune;
	//tunelength = sidFile.tunelength;

	callbackData.initaddr = sidFile.initaddr;
	callbackData.playaddr = sidFile.playaddr;
	callbackData.playaddf = sidFile.playaddf;
	callbackData.SID_address[0] = sidFile.SID_address[0];
	callbackData.SID_address[1] = sidFile.SID_address[1];
	callbackData.SID_address[2] = sidFile.SID_address[2];
	callbackData.SIDamount = sidFile.SIDamount;
	callbackData.requested_SID_model = sidFile.requested_SID_model;
	callbackData.MosSid.SID_model[0] = sidFile.SID_model[0];
	callbackData.MosSid.SID_model[1] = sidFile.SID_model[1];
	callbackData.MosSid.SID_model[2] = sidFile.SID_model[2];

	return 0;
}

int SidPlayer::InserFile(char* fileBuffer, unsigned int sidLength)
{
	sidFile.InserFile(fileBuffer, sidLength);
	sidFile.SetToMemory(callbackData.MosSid.Mos6510.Ram.memory);

	//byte e = Mos6510.Ram.memory[0];

	callbackData.subtune = sidFile.subtune;
	//tunelength = sidFile.tunelength;

	callbackData.initaddr = sidFile.initaddr;
	callbackData.playaddr = sidFile.playaddr;
	callbackData.playaddf = sidFile.playaddf;
	callbackData.SID_address[0] = sidFile.SID_address[0];
	callbackData.SID_address[1] = sidFile.SID_address[1];
	callbackData.SID_address[2] = sidFile.SID_address[2];
	callbackData.SIDamount = sidFile.SIDamount;
	callbackData.requested_SID_model = sidFile.requested_SID_model;
	callbackData.MosSid.SID_model[0] = sidFile.SID_model[0];
	callbackData.MosSid.SID_model[1] = sidFile.SID_model[1];
	callbackData.MosSid.SID_model[2] = sidFile.SID_model[2];

	return 0;
}

int SidPlayer::Play()
{
	callbackData.MosSid.OUTPUT_SCALEDOWN = SID_CHANNEL_AMOUNT * 16 + 26;

	if (callbackData.SIDamount == 2)
		callbackData.MosSid.OUTPUT_SCALEDOWN /= 0.6;
	else if (callbackData.SIDamount >= 3)
		callbackData.MosSid.OUTPUT_SCALEDOWN /= 0.4;


	callbackData.MosSid.cSID_init(callbackData.MosSid.samplerate);
	init(callbackData.subtune);

	sdlHelper.SDL__PauseAudio(0);

	return 0;
}

int SidPlayer::Stop()
{
	sdlHelper.SDL__PauseAudio(1);
	sdlHelper.SDL__CloseAudio();

	return 0;
}


void SidPlayer::init(int subt)
{
	static int timeout;
	callbackData.subtune = subt;
	callbackData.MosSid.Mos6510.initCPU(callbackData.initaddr);
	callbackData.MosSid.initSID();
	callbackData.MosSid.Mos6510.A = callbackData.subtune;
	callbackData.MosSid.Mos6510.Ram.memory[1] = 0x37;
	callbackData.MosSid.Mos6510.Ram.memory[0xDC05] = 0;

	for (timeout = 100000;timeout >= 0;timeout--)
	{
		if (callbackData.MosSid.Mos6510.CPU()) break;
	}
	if (callbackData.timermode[callbackData.subtune] || callbackData.MosSid.Mos6510.Ram.memory[0xDC05])
	{ //&& playaddf {   //CIA timing
		if (!callbackData.MosSid.Mos6510.Ram.memory[0xDC05])
		{
			callbackData.MosSid.Mos6510.Ram.memory[0xDC04] = 0x24;
			callbackData.MosSid.Mos6510.Ram.memory[0xDC05] = 0x40;
		} //C64 startup-default
		callbackData.frame_sampleperiod = (callbackData.MosSid.Mos6510.Ram.memory[0xDC04] + callbackData.MosSid.Mos6510.Ram.memory[0xDC05] * 256) / callbackData.MosSid.clock_ratio;
	}
	else
		callbackData.frame_sampleperiod = callbackData.MosSid.samplerate / PAL_FRAMERATE;  //Vsync timing

	
	#ifdef Print2Console
	//printf("Frame-sampleperiod: %.0f samples  (%.1fX speed)\n", callbackData.MosSid.mos_round(callbackData.frame_sampleperiod), callbackData.MosSid.samplerate / PAL_FRAMERATE / callbackData.frame_sampleperiod);
	sprintf(bufferMessage, "\nFrame-sampleperiod: %.0f samples  (%.1fX speed)\n", callbackData.MosSid.mos_round(callbackData.frame_sampleperiod), callbackData.MosSid.samplerate / PAL_FRAMERATE / callbackData.frame_sampleperiod);
	print2Console(bufferMessage);
	#endif

	//frame_sampleperiod = (memory[0xDC05]!=0 || (!timermode[subtune] && playaddf))? samplerate/PAL_FRAMERATE : (memory[0xDC04] + memory[0xDC05]*256) / clock_ratio; 

	if (callbackData.playaddf == 0)
	{
		callbackData.playaddr = ((callbackData.MosSid.Mos6510.Ram.memory[1] & 3)<2) ? callbackData.MosSid.Mos6510.Ram.memory[0xFFFE] + callbackData.MosSid.Mos6510.Ram.memory[0xFFFF] * 256 : callbackData.MosSid.Mos6510.Ram.memory[0x314] + callbackData.MosSid.Mos6510.Ram.memory[0x315] * 256;
		
		#ifdef Print2Console
		//printf("IRQ-playaddress:%4.4X\n", callbackData.playaddr);
		sprintf(bufferMessage, "IRQ-playaddress:%4.4X\n", callbackData.playaddr);
		print2Console(bufferMessage);
		#endif
	}
	else
	{
		callbackData.playaddr = callbackData.playaddf; if (callbackData.playaddr >= 0xE000 && callbackData.MosSid.Mos6510.Ram.memory[1] == 0x37) callbackData.MosSid.Mos6510.Ram.memory[1] = 0x35;
	} //player under KERNAL (Crystal Kingdom Dizzy)

	callbackData.MosSid.Mos6510.initCPU(callbackData.playaddr); callbackData.framecnt = 1; callbackData.finished = 0; callbackData.CPUtime = 0;
}

void SidPlayer::play(void* voidCallData, Uint8 *stream, int len) //called by SDL at samplerate pace
{
	CallbackData * callData = (static_cast<CallbackData*>(voidCallData));

	static int i, j, output;

	for (i = 0;i<len;i += 2)
	{
		(*callData).framecnt--;
		if ((*callData).framecnt <= 0)
		{
			(*callData).framecnt = (*callData).frame_sampleperiod;
			(*callData).finished = 0;
			(*callData).MosSid.Mos6510.PC = (*callData).playaddr;
			(*callData).MosSid.Mos6510.SP = 0xFF;
		} // printf("%d  %f\n",framecnt,playtime); }

		if ((*callData).finished == 0)
		{
			while ((*callData).CPUtime <= (*callData).MosSid.clock_ratio)
			{
				(*callData).MosSid.Mos6510.pPC = (*callData).MosSid.Mos6510.PC;
				if ((*callData).MosSid.Mos6510.CPU() >= 0xFE || (((*callData).MosSid.Mos6510.Ram.memory[1] & 3)>1 && (*callData).MosSid.Mos6510.pPC<0xE000 && ((*callData).MosSid.Mos6510.PC == 0xEA31 || (*callData).MosSid.Mos6510.PC == 0xEA81)))
				{
					(*callData).finished = 1;
					break;
				}
				else
					(*callData).CPUtime += (*callData).MosSid.Mos6510.cycles; //RTS,RTI and IRQ player ROM return handling


				if (((*callData).MosSid.Mos6510.addr == 0xDC05 || (*callData).MosSid.Mos6510.addr == 0xDC04) && ((*callData).MosSid.Mos6510.Ram.memory[1] & 3) && (*callData).timermode[(*callData).subtune])
				{
					(*callData).frame_sampleperiod = ((*callData).MosSid.Mos6510.Ram.memory[0xDC04] + (*callData).MosSid.Mos6510.Ram.memory[0xDC05] * 256) / (*callData).MosSid.clock_ratio;  //dynamic CIA-setting (Galway/Rubicon workaround)
					if (!(*callData).dynCIA)
					{
						(*callData).dynCIA = 1;

						#ifdef Print2Console
						printf("( Dynamic CIA settings. New frame-sampleperiod: %.0f samples  (%.1fX speed) )\n", (*callData).MosSid.mos_round((*callData).frame_sampleperiod), (*callData).MosSid.samplerate / PAL_FRAMERATE / (*callData).frame_sampleperiod);
						sprintf(bufferMessage, "( Dynamic CIA settings. New frame-sampleperiod: %.0f samples  (%.1fX speed) )\n", (*callData).MosSid.mos_round((*callData).frame_sampleperiod), (*callData).MosSid.samplerate / PAL_FRAMERATE / (*callData).frame_sampleperiod);
						print2Console(bufferMessage);
						#endif
					}
				}

				if ((*callData).MosSid.Mos6510.storadd >= 0xD420 && (*callData).MosSid.Mos6510.storadd<0xD800 && ((*callData).MosSid.Mos6510.Ram.memory[1] & 3))
				{  //CJ in the USA workaround (writing above $d420, except SID2/SID3)
					if (!((*callData).SID_address[1] <= (*callData).MosSid.Mos6510.storadd && (*callData).MosSid.Mos6510.storadd<(*callData).SID_address[1] + 0x1F) && !((*callData).SID_address[2] <= (*callData).MosSid.Mos6510.storadd && (*callData).MosSid.Mos6510.storadd<(*callData).SID_address[2] + 0x1F))
						(*callData).MosSid.Mos6510.Ram.memory[(*callData).MosSid.Mos6510.storadd & 0xD41F] = (*callData).MosSid.Mos6510.Ram.memory[(*callData).MosSid.Mos6510.storadd]; //write to $D400..D41F if not in SID2/SID3 address-space
				}
				if ((*callData).MosSid.Mos6510.addr == 0xD404 && !((*callData).MosSid.Mos6510.Ram.memory[0xD404] & GATE_BITMASK))
					(*callData).MosSid.ADSRstate[0] &= 0x3E; //Whittaker player workarounds (if GATE-bit triggered too fast, 0 for some cycles then 1)
				if ((*callData).MosSid.Mos6510.addr == 0xD40B && !((*callData).MosSid.Mos6510.Ram.memory[0xD40B] & GATE_BITMASK))
					(*callData).MosSid.ADSRstate[1] &= 0x3E;
				if ((*callData).MosSid.Mos6510.addr == 0xD412 && !((*callData).MosSid.Mos6510.Ram.memory[0xD412] & GATE_BITMASK))
					(*callData).MosSid.ADSRstate[2] &= 0x3E;
			}

			(*callData).CPUtime -= (*callData).MosSid.clock_ratio;
		}

		output = (*callData).MosSid.SID(0, 0xD400);


		if ((*callData).SIDamount >= 2)
			output += (*callData).MosSid.SID(1, (*callData).SID_address[1]);
		if ((*callData).SIDamount == 3)
			output += (*callData).MosSid.SID(2, (*callData).SID_address[2]);

		stream[i] = output & 0xFF; 
		stream[i + 1] = output >> 8;
	}
}
