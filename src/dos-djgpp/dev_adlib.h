/* The original version of this file was written by Kevin A. Lee */

#if !defined(_dev_adlib_h)
#define _dev_adlib_h

#define MIN_REGISTER        0x01
#define MAX_REGISTER        0xF5
#define ADLIB_FM_ADDRESS    0x388       /* adlib address/status register */
#define ADLIB_FM_DATA       0x389       /* adlib data register           */

#ifndef BYTE
#define BYTE unsigned char
#endif

typedef struct
{BYTE MOD_KSL,MOD_fMult,MOD_attack,MOD_sustain,MOD_ss;
 BYTE MOD_decay,MOD_release,MOD_outputLevel;
 BYTE MOD_amplitudeVibrato,MOD_frequencyVibrato,MOD_envelopeScaling;

 BYTE CAR_KSL,CAR_fMult,CAR_FB,CAR_attack,CAR_sustain,CAR_ss;
 BYTE CAR_decay,CAR_release,CAR_outputLevel;
 BYTE CAR_amplitudeVibrato,CAR_frequencyVibrato,CAR_envelopeScaling;

 BYTE FM,MOD_waveForm,CAR_waveForm,feedBack,PADDING;
}  FMInstrument;


/* function prototyping */
void WriteFM(int reg, int value);
int  ReadFM(void);
int  AdlibExists(void);

void FMReset(int percusiveMode);
/* Resets the card, quiets all voices, sets the percussive mode state */

void FMKeyOff(int voice);
/* Percussion is handled correctly */

void FMKeyOn(int voice, int freq);
/* Percussion is handled correctly */

void FMVoiceVolume(int voice, int vol);
/* Never tested this, probably doesn't work. */

void FMSetVoice(int voiceNum, FMInstrument *ins);
/* Remember: percussion instruments must be assigned only to the
correct voice number. */

void FMSetPercusiveMode(int state);

////////////////////////////////////////////////////////////////
// SB

#include "drvsound.h"

#endif


