#ifndef MOS_SID_H
#define MOS_SID_H

#include <math.h>

#include "MOS_6510.h"

//----------------------------- SID emulation -----------------------------------------

#define C64_PAL_CPUCLK 985248.0
#define DEFAULT_SAMPLERATE 44100.0         //(Soldier of Fortune, 2nd Reality, Alliance, X-tra energy, Jackal, Sanxion, Ultravox, Hard Track, Swing, Myth, LN3, etc.)
#define CLOCK_RATIO_DEFAULT C64_PAL_CPUCLK/DEFAULT_SAMPLERATE  //(50.0567520: lowest framerate where Sanxion is fine, and highest where Myth is almost fine)
#define PERIOD0 CLOCK_RATIO_DEFAULT //max(round(clock_ratio),9)
#define STEP0 3 //ceil(PERIOD0/9.0)
#define PAL_FRAMERATE 50.06 //50.0443427 //50.1245419 //(C64_PAL_CPUCLK/63/312.5), selected carefully otherwise some ADSR-sensitive tunes may suffer more:
#define CAP_6581 0.470 //nF //filter capacitor value for 6581
#define FILTER_DARKNESS_6581 22.0 //the bigger the value, the darker the filter control is (that is, cutoff frequency increases less with the same cutoff-value)
#define VCR_FET_TRESHOLD 192 //Vth (on cutoff numeric range 0..2048) for the VCR cutoff-frequency control FET below which it doesn't conduct
#define SID_CHANNEL_AMOUNT 3 
#define VCR_SHUNT_6581 1500 //kOhm //cca 1.5 MOhm Rshunt across VCR FET drain and source (causing 220Hz bottom cutoff with 470pF integrator capacitors in old C64)
#define FILTER_DISTORTION_6581 0.0016 //the bigger the value the more of resistance-modulation (filter distortion) is applied for 6581 cutoff-control


enum {
	GATE_BITMASK = 0x01, SYNC_BITMASK = 0x02, RING_BITMASK = 0x04, TEST_BITMASK = 0x08,
	TRI_BITMASK = 0x10, SAW_BITMASK = 0x20, PULSE_BITMASK = 0x40, NOISE_BITMASK = 0x80,
	HOLDZERO_BITMASK = 0x10, DECAYSUSTAIN_BITMASK = 0x40, ATTACK_BITMASK = 0x80,
	LOWPASS_BITMASK = 0x10, BANDPASS_BITMASK = 0x20, HIGHPASS_BITMASK = 0x40, OFF3_BITMASK = 0x80
};


class MOS_SID
{
public:
	MOS_SID();
	MOS_SID(MOS_6510 & m6);
	~MOS_SID();

	void cSID_init(int samplerate);
	void initSID();

	int SID(char num, unsigned int baseaddr);

	int mos_round(double d);

	MOS_6510 Mos6510;

	int SID_model[3];
	byte ADSRstate[9];

	float clock_ratio;
	int samplerate;
	int OUTPUT_SCALEDOWN;

private:

	unsigned int combinedWF(char num, char channel, unsigned int* wfarray, int index, char differ6581, byte freq);
	void createCombinedWF(unsigned int* wfarray, float bitmul, float bitstrength, float treshold);
	
	unsigned int TriSaw_8580[4096], PulseSaw_8580[4096], PulseTriSaw_8580[4096];

	float ADSRperiods[16];
	byte ADSRstep[16];
	static const byte ADSR_exptable[256];

	unsigned int prevwavdata[9];
	byte expcnt[9];
	float ratecnt[9];
	short int envcnt[9];
	static const byte FILTSW[9];

	float cutoff_ratio_8580, cutoff_steepness_6581, cap_6581_reciprocal; //, cutoff_ratio_6581, cutoff_bottom_6581, cutoff_top_6581;
																		 //player-related variables:
	unsigned int prevwfout[9], sourceMSB[3], noise_LFSR[9];
	int phaseaccu[9], prevaccu[9], prevlowpass[3], prevbandpass[3];
	byte prevSR[9], sourceMSBrise[9];

};

#endif