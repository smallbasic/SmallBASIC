/* The original version of this file was written by Kevin A. Lee */
/* (all the bugs are HIS fault. (just kidding)) */

#include "sys.h"
#include "drvsound.h"

#include <conio.h>
#include <time.h>
#include <dos.h>
#include "dev_adlib.h"
#define	 TPS	CLOCKS_PER_SEC

/**WARNING**WARNING**WARNING**WARNING**WARNING**WARNING**WARNING***\
 * This module MUST be compiled with stack checking OFF in BC 3.0 *
\**WARNING**WARNING**WARNING**WARNING**WARNING**WARNING**WARNING***/

/* We wish that THIS would turn stack checking off, but it doesn't work */
#pragma -N-
   
static unsigned char currentB0Contents[9]={0,0,0,0,0,0,0,0,0};
static int percussiveMode=0,currentBDContents=0;
static int opTable[18]={0,1,2,3,4,5,8,9,10,11,12,13,16,17,18,19,20,21};
static int voiceModulator[11]={0,1,2,6,7,8,12,16,14,17,13};
static int voiceCarrier[9]={3,4,5,9,10,11,15,16,17};

/* local function */
void Wait(clock_t wait);

void WriteFM(int reg, int value)
{int i;
 outp(ADLIB_FM_ADDRESS, (BYTE)reg);              /* set up the register  */
 for (i = 0; i < 6; i++) inp(ADLIB_FM_ADDRESS);  /* wait 12 cycles       */
 outp(ADLIB_FM_DATA, (BYTE)value);               /* write out the value  */
 for (i = 0; i < 35; i++) inp(ADLIB_FM_ADDRESS); /* wait 84 cycles       */
}

int ReadFM(void)
{return (inp(ADLIB_FM_ADDRESS));
}

int AdlibExists(void)
{int stat1, stat2;
 WriteFM(0x04, 0x60);            /* reset both timers        */
 WriteFM(0x04, 0x80);            /* enable timer interrupts  */
 stat1 = ReadFM();               /* read status register     */
 WriteFM(0x02, 0xFF);
 WriteFM(0x04, 0x21);            /* start timer 1            */
 Wait(80);                       /* could do something useful*/
 stat2 = ReadFM();               /* read status register     */
 WriteFM(0x04, 0x60);            /* reset both timers        */
 WriteFM(0x04, 0x80);            /* enable timer interrupts  */
 
 if (((stat1 & 0xE0) == 0x00) && ((stat2 & 0xE0) == 0xC0)) return (1);
 return (0);
}

void FMReset(int percusiveMode)
{
 int i;
 /* zero all registers */
 for (i = MIN_REGISTER; i < MAX_REGISTER+1; i++) WriteFM(i, 0);
 /* allow FM chips to control the waveform of each operator */
 WriteFM(0x01, 0x20);
 FMSetPercusiveMode(percusiveMode);
}

void FMSetPercusiveMode(int state)
{if (state)
    {WriteFM(0xBD, 0x20);
     currentBDContents=0x20;
     percussiveMode=1;
     voiceModulator[7]=16;
     voiceModulator[8]=14;
     /* we have to set the freq of voice 7 & 8 for the white noise gen.*/
     /* these frequency choices could certainly be better */
     WriteFM(0xa7, 1844 & 0xff);
     WriteFM(0xb7, 1844 >> 8);
     WriteFM(0xa8, 3764 & 0xff);
     WriteFM(0xb8, 3764 >> 8);
    }
else
   {WriteFM(0xBD, 0);
    percussiveMode=0;
    currentBDContents=0;
    voiceModulator[7]=13;
    voiceModulator[8]=14;
   }
}

void FMKeyOff(int voice)
{int regNum;
 /* turn voice off */
 if (percussiveMode && voice>5)
    {currentBDContents&=(0xFF-(16 >> (voice-6)));
     WriteFM(0xBD,currentBDContents);
    }
 else
    {regNum = 0xB0 + voice;
     WriteFM(regNum, currentB0Contents[voice] & 0xdf);
     /* must preserve frequency for release stage of envelope */
    }
}

void FMKeyOn(int voice, int freq)
{
 int regNum, tmp;
 if (percussiveMode && voice>5)
    {currentBDContents|=16 >> (voice-6);
     if (voice==6 || voice==8) /* bass and tom-tom are pitched */
	{regNum = 0xA0 + voice;
	 WriteFM(regNum, freq & 0xff);
	 regNum = 0xB0 + voice;
	 tmp = (freq >> 8);
	 WriteFM(regNum, tmp);
	}
     WriteFM(0xBD,currentBDContents);
    }
 else
    {regNum = 0xA0 + voice;
     WriteFM(regNum, freq & 0xff);
     regNum = 0xB0 + voice;
     tmp = (freq >> 8) | 0x20;
     WriteFM(regNum, tmp);
     currentB0Contents[voice]=tmp;
    }
}

void FMVoiceVolume(int voice, int vol)
{int regNum;
 regNum = 0x40+opTable[voiceModulator[voice]];
 WriteFM(regNum, vol); /* must preserve KSL (but don't yet!) */
}

void FMSetVoice(int voiceNum, FMInstrument *ins)
{int modO,carO,i;
 if (voiceNum<7 || !percussiveMode) /* note: base drum included */
    {modO=opTable[voiceModulator[voiceNum]];
     carO=opTable[voiceCarrier[voiceNum]];
     WriteFM(0x20+modO,
	     (ins->MOD_amplitudeVibrato << 7) +
	     (ins->MOD_frequencyVibrato << 6) +
	     (ins->MOD_ss << 5)               +
	     (ins->MOD_envelopeScaling << 4)  +
	     (ins->MOD_fMult));
     WriteFM(0x40+modO,
	     (ins->MOD_KSL << 6) +
	     (ins->MOD_outputLevel));
     WriteFM(0x60+modO,
	     (ins->MOD_attack << 4) + (ins->MOD_decay));
     WriteFM(0x80+modO,
	     (ins->MOD_sustain << 4) + (ins->MOD_release));
     WriteFM(0xe0+modO,ins->MOD_waveForm);
     
     WriteFM(0x20+carO,
	     (ins->CAR_amplitudeVibrato << 7) +
	     (ins->CAR_frequencyVibrato << 6) +
	     (ins->CAR_ss << 5)               +
	     (ins->CAR_envelopeScaling << 4)  +
	     (ins->CAR_fMult));
     WriteFM(0x40+carO,
	     (ins->CAR_KSL << 6) +
	     (ins->CAR_outputLevel));
     WriteFM(0x60+carO,
	     (ins->CAR_attack << 4) + (ins->CAR_decay));
     WriteFM(0x80+carO,
	     (ins->CAR_sustain << 4) + (ins->CAR_release));
     WriteFM(0xe0+carO,ins->CAR_waveForm);
     if (ins->FM) /* is the adlib documentation fucked or what? */
	i=0;
     else
	i=1;
     WriteFM(0xC0+voiceNum,(ins->feedBack << 1) + i);
    }
 else
    {modO=opTable[voiceModulator[voiceNum]];
     WriteFM(0x20+modO,
	     (ins->MOD_amplitudeVibrato << 7) +
	     (ins->MOD_frequencyVibrato << 6) +
	     (ins->MOD_ss << 5)               +
	     (ins->MOD_envelopeScaling << 4)  +
	     (ins->MOD_fMult));
     WriteFM(0x40+modO,
	     (ins->MOD_KSL << 6) +
	     (ins->MOD_outputLevel));
     WriteFM(0x60+modO,
	     (ins->MOD_attack << 4) + (ins->MOD_decay));
     WriteFM(0x80+modO,
	     (ins->MOD_sustain << 4) + (ins->MOD_release));
     WriteFM(0xe0+modO,ins->MOD_waveForm);
    }
}

void Wait(clock_t wait)
{clock_t goal;
 if (!wait) return;
 goal = wait + clock();
 while ((goal > clock())) ;
}


////////////////////////////////////////////////////////////////////////
//	SB

static dbt_t audio_queue;
static int	audio_qhead;
static int	audio_qtail;
struct audio_node_s	{
	word	frq;
	word	dur;
	byte	vol;
	dword	start;
	dword	end;
	byte	status;
	};
typedef struct audio_node_s audio_node;
#define	AUDIO_QSIZE		256

int		drvsound_init()
{
	if	( AdlibExists() )	{
		FMReset(0);
		audio_queue = dbt_create("vAUDIO", 0);
		dbt_prealloc(audio_queue, AUDIO_QSIZE, sizeof(audio_node));
		audio_qhead = audio_qtail = 0;
		return 1;
		}
	return 0;
}

void	drvsound_close()
{
	audio_qhead = audio_qtail = 0;
	dbt_close(audio_queue);
	FMReset(0);
}

/*
*/
void	drvsound_realsound(int frq, int ms, int vol)
{
	if	( frq )	{
		FMKeyOn(0, frq);
		FMVoiceVolume(0, vol);	// ranges of vol ?
		}
	else	
		FMKeyOff(0);
}

/*
*/
void	drvsound_backsound(dword dif)
{
	audio_node	node;
	dword		now;

	if	( audio_qhead != audio_qtail )	{
		dbt_read(audio_queue, audio_qhead, &node, sizeof(audio_node));

		if	( node.status == 1 )	{	// I am playing
			now = clock();
			if	( now >= node.end )	{
				drvsound_realsound(0, 0, 0);	// stop

				audio_qhead ++;
				if	( audio_qhead >= AUDIO_QSIZE )
					audio_qhead = 0;

				if	( now > node.end )
					drvsound_backsound(now - node.end);	// read next NOW
				else
					drvsound_backsound(0);	// read next NOW
				}
			}
		else	{	// next cmd
			node.start = clock() + dif;
			node.end = node.start + ((node.dur * TPS) / 1000);

			if	( node.frq )
				drvsound_realsound(node.frq, node.dur, node.vol);	// start play
			else
				drvsound_realsound(0, 0, 0);	// stop

			node.status = 1;
			dbt_write(audio_queue, audio_qhead, &node, sizeof(audio_node));
			}
		}
}

void	drvsound_sound(int frq, int ms, int vol, int bgplay)
{
	if	( !bgplay )	{
		drvsound_realsound(frq, ms, vol);
		Wait((ms * CLOCKS_PER_SEC) / 1000);
		}
	else	{
		audio_node	node;

		node.status = 0;
		node.frq = frq;
		node.dur = ms;
		node.vol = vol;
		dbt_write(audio_queue, audio_qtail, &node, sizeof(audio_node));
		
		audio_qtail ++;
		if ( audio_qtail >= AUDIO_QSIZE )
			audio_qtail = 0;
		}
}

void	drvsound_clear_queue()
{
	audio_qhead = audio_qtail;
}

void	drvsound_beep()
{
	drvsound_sound(440, 125, 75, 0);
}

void	drvsound_event()
{
	drvsound_backsound(0);
}

