/*
*	Windows sound driver
*
*	This code is based on Yannick Sustrac (yannstrc@mail.dotcom.fr) code
*/

#include <vcl.h>
#include <math.h>
#include <process.h>
#include <mmsystem.h>

#include "drvsound.h"

volatile int audio_init = 0;
static bool use_sound_driver = true;
static bool use_sound = true;

typedef struct {
	int		frq;
	__int64	dur;
	int		vol;
	__int64	start;
	__int64	end;
	int		status;
	} audio_node;

#define	AUDIO_QSIZE		4096
static audio_node	audio_queue[AUDIO_QSIZE];
static int	audio_qhead;
static int	audio_qtail;

DWORD WINAPI	ossdsp_backg(LPVOID);
static DWORD	audio_thread_id;
static HANDLE	audio_thread_handle;

#define	AfxMessageBox(x)

#define MAX_OUTPUT_SAMPLES 4096
#define MAX_VOIE 2
#define MAX_SIZE_SAMPLES  1  // WORD
#define MAX_SIZE_OUTPUT_BUFFER   MAX_OUTPUT_SAMPLES*MAX_VOIE*MAX_SIZE_SAMPLES

#define real double
#define CALL_BACK_TEST
DWORD WINAPI WaveOutThreadProc(void * pParam);

/*
*	These sources are taken by an sound app (www.codeguru.com)
*	The original notes of the author are following (good job Yannick)
*/

/*

	    This program is Copyright  Developped by Yannick Sustrac
                   yannstrc@mail.dotcom.fr
		        http://www.mygale.org/~yannstrc/


This program is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program;
if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

class CSoundOut
{
public:

	SHORT  OutputBuffer[2][MAX_SIZE_OUTPUT_BUFFER];

	WAVEOUTCAPS		m_WaveOutDevCaps;
    HWAVEOUT		m_WaveOut;
	WAVEHDR			m_WaveHeader[2];
    WAVEFORMATEX	m_WaveFormat;
	
	HANDLE 			m_WaveOutEvent;
//	CWinThread * m_WaveOutThread;
	HANDLE		m_WaveOutThread;
	BOOL m_Terminate;

	UINT m_WaveOutSampleRate;
	int  m_NbMaxSamples;
	UINT m_SizeRecord;


//////////////////////////////////////////////////////
// functions members

public :
	void StartOutput();
	void StopOutput();
	void CloseOutput();
	void AddBuffer();

public :
	virtual void RazBuffers();
    virtual void ComputeSamples(SHORT *);
	

	MMRESULT OpenOutput();
	void WaveInitFormat(    WORD    nCh,		 // number of channels (mono, stereo)
							DWORD   nSampleRate, // sample rate
							WORD    BitsPerSample);
	
	CSoundOut();
	virtual ~CSoundOut();

	SHORT m_Toggle;
};

DWORD WINAPI WaveOutThreadProc(void * pParam);
void CALLBACK WaveOutProc(HWAVEOUT hwo,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2);

// Construction/Destruction
CSoundOut::CSoundOut()
{
  m_NbMaxSamples = MAX_OUTPUT_SAMPLES;
  m_WaveOutSampleRate = 11025;

  m_Toggle = 0;
}

CSoundOut::~CSoundOut()
{
	CloseOutput();
}

MMRESULT CSoundOut::OpenOutput()
{
     MMRESULT result;

    result=waveOutGetNumDevs();
	if (result == 0)
	{
        AfxMessageBox("No Sound Output Device");
		return result;
	}

   // test for Mic available
   result=waveOutGetDevCaps (0, &m_WaveOutDevCaps, sizeof(WAVEOUTCAPS));
   
   if ( result!= MMSYSERR_NOERROR)
   {
       AfxMessageBox(_T("Sound output Cannot determine card capabilities !"));
   }

	m_Terminate = FALSE;
#ifndef CALL_BACK_TEST
	// The SoundOut Devive is OK now we can create an Event  and start the Thread
	m_WaveOutEvent = CreateEvent(NULL,FALSE,FALSE,"WaveOutThreadEvent");
//	m_WaveOutThread= AfxBeginThread(WaveOutThreadProc,this,THREAD_PRIORITY_TIME_CRITICAL,0,CREATE_SUSPENDED,NULL);
	DWORD	thread_id;
	m_WaveOutThread= CreateThread(NULL, 0, WaveOutThreadProc, NULL, CREATE_SUSPENDED, &thread_id);
//	m_WaveOutThread->m_bAutoDelete = TRUE;
#endif
// start the thread at the end of the buffer init
	// init format 
	WaveInitFormat(1/* mono*/,m_WaveOutSampleRate /* khz */,16 /* bits */); 

	// Open Output 
#ifndef CALL_BACK_TEST
	result = waveOutOpen( &m_WaveOut,0, &m_WaveFormat,(DWORD)m_WaveOutEvent ,NULL ,CALLBACK_EVENT);
#else
	result = waveOutOpen( &m_WaveOut,0, &m_WaveFormat,(DWORD)WaveOutProc ,(ULONG)this ,CALLBACK_FUNCTION);
#endif

	if ( result!= MMSYSERR_NOERROR)
	{
        AfxMessageBox(_T("Sound output Cannot Open Device!"));
	    return result;
	}


	m_Toggle = 0;
	m_SizeRecord = m_NbMaxSamples;
    m_WaveHeader[m_Toggle].lpData = (CHAR *)&OutputBuffer[m_Toggle][0];
    m_WaveHeader[m_Toggle].dwBufferLength = m_SizeRecord*2;
	m_WaveHeader[m_Toggle].dwFlags = 0;

    result = waveOutPrepareHeader( m_WaveOut, &m_WaveHeader[m_Toggle], sizeof(WAVEHDR) ); 
  //MMRESULT waveOutPrepareHeader( HWAVEOUT hwi, LPWAVEHDR pwh, UINT cbwh ); 
   if ( (result!= MMSYSERR_NOERROR) || ( m_WaveHeader[m_Toggle].dwFlags != WHDR_PREPARED) )
   {
        AfxMessageBox(_T(" Sound Output Cannot Prepare Header !"));
	    return result;
   }

   result = waveOutWrite( m_WaveOut, &m_WaveHeader[m_Toggle], sizeof(WAVEHDR) );
   if  (result!= MMSYSERR_NOERROR) 
   {
        AfxMessageBox(_T(" Sound Output Cannot Write Buffer !"));
	    return result;
   }


   // register the second frame don't wait for the end of the first one
   // so when we will be notified, this second frame will be currently output when we will reload the first one
	m_Toggle = 1;
	m_SizeRecord = m_NbMaxSamples;
    m_WaveHeader[m_Toggle].lpData = (CHAR *)&OutputBuffer[m_Toggle][0];
    m_WaveHeader[m_Toggle].dwBufferLength = m_SizeRecord*2;
	m_WaveHeader[m_Toggle].dwFlags = 0;

    result = waveOutPrepareHeader( m_WaveOut, &m_WaveHeader[m_Toggle], sizeof(WAVEHDR) ); 
  //MMRESULT waveOutPrepareHeader( HWAVEOUT hwi, LPWAVEHDR pwh, UINT cbwh );
   if ( (result!= MMSYSERR_NOERROR) || ( m_WaveHeader[m_Toggle].dwFlags != WHDR_PREPARED) )
   {
        AfxMessageBox(_T(" Sound Output Cannot Prepare Header !"));
	    return result;
   }

   result = waveOutWrite( m_WaveOut, &m_WaveHeader[m_Toggle], sizeof(WAVEHDR) );
   if  (result!= MMSYSERR_NOERROR)
   {
        AfxMessageBox(_T(" Sound Output Cannot Write Buffer !"));
	    return result;
   }

#ifndef CALL_BACK_TEST
//	m_WaveOutThread->ResumeThread();
	SetThreadPriority(m_WaveOutThread, THREAD_PRIORITY_HIGHEST);
	ResumeThread(m_WaveOutThread);
	CloseHandle(m_WaveOutThread);
#endif

   return result;
}

void CSoundOut::AddBuffer()
{
	
   MMRESULT result;

   if (m_Toggle == 0) 
	   m_Toggle = 1; 
   else
	   m_Toggle = 0;
   result = waveOutUnprepareHeader( m_WaveOut, &m_WaveHeader[m_Toggle], sizeof(WAVEHDR) );
   if  (result!= MMSYSERR_NOERROR)
   {
        AfxMessageBox(_T("Sound output Cannot UnPrepareHeader !"));
        return;
   };
 	m_SizeRecord = m_NbMaxSamples;
    m_WaveHeader[m_Toggle].lpData = (CHAR *)&OutputBuffer[m_Toggle][0];
    m_WaveHeader[m_Toggle].dwBufferLength = m_SizeRecord *2;
	m_WaveHeader[m_Toggle].dwLoops = 0;
	m_WaveHeader[m_Toggle].dwFlags = 0; //WHDR_BEGINLOOP ;

    result = waveOutPrepareHeader( m_WaveOut, &m_WaveHeader[m_Toggle], sizeof(WAVEHDR) );
   if ( (result!= MMSYSERR_NOERROR) || ( m_WaveHeader[m_Toggle].dwFlags != WHDR_PREPARED) )
        AfxMessageBox(_T("Sound output Cannot Prepare Header !"));

   result = waveOutWrite( m_WaveOut, &m_WaveHeader[m_Toggle], sizeof(WAVEHDR) );
   if  (result!= MMSYSERR_NOERROR)
        AfxMessageBox(_T("Sound output Cannot Add Buffer !"));

}


/*
WAVE_FORMAT_1M08  11.025 kHz, mono, 8-bit 
WAVE_FORMAT_1M16  11.025 kHz, mono, 16-bit 
WAVE_FORMAT_1S08  11.025 kHz, stereo, 8-bit 
WAVE_FORMAT_1S16  11.025 kHz, stereo, 16-bit 
WAVE_FORMAT_2M08  22.05 kHz, mono, 8-bit
WAVE_FORMAT_2M16  22.05 kHz, mono, 16-bit 
WAVE_FORMAT_2S08  22.05 kHz, stereo, 8-bit
WAVE_FORMAT_2S16  22.05 kHz, stereo, 16-bit 
WAVE_FORMAT_4M08  44.1 kHz, mono, 8-bit 
WAVE_FORMAT_4M16  44.1 kHz, mono, 16-bit
WAVE_FORMAT_4S08  44.1 kHz, stereo, 8-bit 
WAVE_FORMAT_4S16  44.1 kHz, stereo, 16-bit 
*/ 

void CSoundOut:: WaveInitFormat(    WORD    nCh, // number of channels (mono, stereo)
								DWORD   nSampleRate, // sample rate
								WORD    BitsPerSample)
{
		m_WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
		m_WaveFormat.nChannels = nCh;
		m_WaveFormat.nSamplesPerSec = nSampleRate;
		m_WaveFormat.nAvgBytesPerSec = nSampleRate * nCh * BitsPerSample/8;
		m_WaveFormat.nBlockAlign = m_WaveFormat.nChannels * BitsPerSample/8;
		m_WaveFormat.wBitsPerSample = BitsPerSample;
		m_WaveFormat.cbSize = 0;
}


///////////////////////////////////////////////////////////////////////////
// the comutation for the Output samples need to be calibrated according
// to the SoundOut board  add an Offset and a Mult coef.

void CSoundOut::ComputeSamples(SHORT *pt)
{
}  


void CSoundOut::CloseOutput()
{
	if (m_WaveOut)
		waveOutPause(m_WaveOut);

	m_Terminate = TRUE;

   if (m_WaveOut)   {
		waveOutReset(m_WaveOut);
		waveOutClose(m_WaveOut);
   }
}

void CSoundOut::RazBuffers()
{
		for (int i=0;i<MAX_OUTPUT_SAMPLES;i++)
		{
	        OutputBuffer[0][i] = 0;
	        OutputBuffer[1][i] = 0;
		}
}

void CSoundOut::StopOutput()
{
	waveOutPause(m_WaveOut);
}

void CSoundOut::StartOutput()
{
	waveOutRestart(m_WaveOut);
}

//    Glogal Thread procedure for the CSoundOut class
//    It cannot be included inside the Class
//
// The LPARAM is the Class pointer it can be the base class CSoundOut
// or a drived class like CFft
// The value of this parametre can change according because
//  OpenMic() call a virtual funtion

#define PT_S ((CSoundOut*)pParam)

#ifndef CALL_BACK_TEST
DWORD WINAPI WaveOutThreadProc(void * pParam)
{
   UINT result;
   UINT FirstPass = TRUE;


	if ( FirstPass)
		result = WaitForSingleObject(PT_S->m_WaveOutEvent,INFINITE);
	FirstPass = FALSE;

	while (!PT_S->m_Terminate)
	{
		result = WaitForSingleObject(PT_S->m_WaveOutEvent,INFINITE);
		if ((result == WAIT_OBJECT_0)&&(!PT_S->m_Terminate ))
		{
			PT_S->AddBuffer();      // Toggle as changed state here !Toggle point to the just received buffer
		    PT_S->ComputeSamples(&PT_S->m_Toggle);
		}
		else
			return 0;  //
	}
    return 0;
}
#endif

#ifdef CALL_BACK_TEST

void CALLBACK WaveOutProc(HWAVEOUT hwo,UINT uMsg,DWORD pParam,DWORD dwParam1,DWORD dwParam2)
{
	if (!PT_S->m_Terminate )
	switch (uMsg)
	{
	case WOM_DONE:
		PT_S->AddBuffer();
	    PT_S->ComputeSamples(&PT_S->m_Toggle);
		break;
	case WOM_OPEN:
		break;
	case WOM_CLOSE:
		break;
	}
}
#endif

///////////////////////////////////////////////////////
class CSinGenerator : public CSoundOut
{
public:
	double fo ;
	double fs ;
	double wo ;
	SHORT Ampl;
	double m_2cosWo;
	double  OutputBufferR[2][MAX_OUTPUT_SAMPLES];

public:
	void Restart();
	void SetSinParametres(SHORT A,  double ferq);
	virtual void ComputeSamples(SHORT *);
	void SetInitialConditions();
	CSinGenerator();
	virtual ~CSinGenerator();

    void	SBStart(int frq, int amp);
    void	SBStop();
	};

CSinGenerator::CSinGenerator()
{
/*
	double pi = 4.0*atan(1.0);
	fo = 300;
	fs = (double)m_WaveOutSampleRate;
	wo = 2*pi*fo/fs;
	Ampl = 10000;
	m_2cosWo =2*cos(wo);

	SetInitialConditions();
	m_Toggle = 0;
	ComputeSamples(NULL);
	m_Toggle = 1;
	ComputeSamples(NULL);
*/
	fs = (double)m_WaveOutSampleRate;
}

CSinGenerator::~CSinGenerator()
{

}


void CSinGenerator::SetInitialConditions()
{
	OutputBufferR[1][m_NbMaxSamples-2]= (double) (Ampl * -sin(wo)); 
	OutputBufferR[1][m_NbMaxSamples-1]= 0;        
}



void CSinGenerator::ComputeSamples(SHORT *unused)
{
   if (m_Toggle)
   {
		OutputBufferR[1][0] = (double)( m_2cosWo*OutputBufferR[0][m_NbMaxSamples-1]-OutputBufferR[0][m_NbMaxSamples-2]);
		OutputBuffer[1][0] = (SHORT)OutputBufferR[1][0];
		OutputBufferR[1][1] = (double)( m_2cosWo*OutputBufferR[1][0]-OutputBufferR[0][m_NbMaxSamples-1]);
		OutputBuffer[1][1] = (SHORT)OutputBufferR[1][1];
		for (int i = 2; i< m_NbMaxSamples; i++)
		{
			OutputBufferR[1][i] = (double)( m_2cosWo*OutputBufferR[1][i-1]-OutputBufferR[1][i-2]);
			OutputBuffer[1][i] = (SHORT)OutputBufferR[1][i];
		}
   }
	else
	{
		OutputBufferR[0][0] = (double)( m_2cosWo*OutputBufferR[1][m_NbMaxSamples-1]-OutputBufferR[1][m_NbMaxSamples-2]);
		OutputBuffer[0][0] = (SHORT)OutputBufferR[0][0];
		OutputBufferR[0][1] = (double)( m_2cosWo*OutputBufferR[0][0]-OutputBufferR[1][m_NbMaxSamples-1]);
		OutputBuffer[0][1] = (SHORT)OutputBufferR[0][1];

		for (int i = 2; i< m_NbMaxSamples; i++)
		{
			OutputBufferR[0][i] = (double)( m_2cosWo*OutputBufferR[0][i-1]-OutputBufferR[0][i-2]);
			OutputBuffer[0][i] = (SHORT)OutputBufferR[0][i];
		}
	}
}

void CSinGenerator::SetSinParametres(SHORT A, double freq)
{
  Ampl = A;
  fo= freq;
}

void CSinGenerator::Restart()
{
	double pi = 4.0*atan(1.0);

	CloseOutput();
	wo = 2*pi*fo/fs;
	m_2cosWo =2*cos(wo);

	SetInitialConditions();
	m_Toggle = 0;
	ComputeSamples(NULL);
	m_Toggle = 1;
	ComputeSamples(NULL);
	OpenOutput();
}

void CSinGenerator::SBStart(int frq, int amp)
{
//	double pi = 4.0*atan(1.0);

	Ampl = amp;
	fo = frq;

/*
	wo = 2*pi*fo/fs;
	m_2cosWo = 2*cos(wo);

	SetInitialConditions();
	m_Toggle = 0;
	ComputeSamples(NULL);
	m_Toggle = 1;
	ComputeSamples(NULL);
	OpenOutput();
*/
	Restart();
}

void CSinGenerator::SBStop()
{
	CloseOutput();
}

/* ------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------- */

CSoundOut		devsnd;
CSinGenerator	devssg;

/*
*/
int _cdecl	drvsound_init()
{
	if	( use_sound_driver )	{
		if	( devssg.OpenOutput() != 0 )	{
			audio_init = 0;
			return 0;
			}

		audio_qhead = audio_qtail = 0;
		audio_init = 1;
		}

	return 1;
}

/*
*/
void _cdecl drvsound_close()
{
	if	( use_sound_driver )	{
		if	( audio_init )	{
			audio_init = 0;
			audio_qhead = audio_qtail;	// clear queue
			devssg.CloseOutput();
			}
		}
}

/*
*	play a tone
*/
void osd_realsound(int freq, int time_ms, int vol)
{
	if	( use_sound_driver )	{
		if	( !audio_init )
			return;

		if	( freq )
			devssg.SBStart(freq, vol * 160);
		else
			devssg.SBStop();
		}			
}

/*
*/
void	osd_backsound(__int64 dif)
{
	if	( use_sound_driver )	{
		audio_node	*node;
		__int64		now, tps;

		if	( audio_qhead != audio_qtail )	{
			node = &audio_queue[audio_qhead];
			QueryPerformanceCounter((LARGE_INTEGER *) &now);

			if	( node->status == 1 )	{	// I am playing
				if	( now >= node->end )	{
					audio_qhead ++;
					if	( audio_qhead >= AUDIO_QSIZE )
						audio_qhead = 0;

					osd_backsound(now - node->end);	// read next NOW
					}
				}
			else	{	// next cmd
				QueryPerformanceFrequency((LARGE_INTEGER *) &tps);

				node->start = now + dif;
				node->end = node->start + ((node->dur * tps) / 1000);

				if	( node->frq )
					osd_realsound(node->frq, node->dur, node->vol);	// start play
				else
					devssg.StopOutput();

				node->status = 1;
				}
			}
		else
			devssg.CloseOutput();
		}
}

/*
*	store sound to audio-list
*/
void _cdecl drvsound_sound(int frq, int  ms, int vol, int bgplay)
{
	if	( use_sound_driver )	{
		audio_node	*node;

		if	( audio_init )	{
			if	( bgplay )	{
				node = &audio_queue[audio_qtail];

				node->status = 0;
				node->frq = frq;
				node->dur = ms;
				node->vol = vol;

				audio_qtail ++;
				if ( audio_qtail >= AUDIO_QSIZE )
					audio_qtail = 0;

				if	( !bgplay )
					Sleep(ms);
				}
			else	{
				devssg.SBStart(frq, vol * 160);
				Sleep(ms);
				devssg.CloseOutput();
				}
			}
		}
	else if ( !bgplay )	{
		if	( use_sound )
			::Beep(frq, ms);
		}
}

/*
*	BEEP :)
*/
void _cdecl drvsound_beep()
{
	drvsound_sound(440, 250, 75, 0);
}

/*
*/
void _cdecl drvsound_event()
{
	if	( use_sound_driver )
		osd_backsound(0);
}

/*
*	stop sound & clear queue
*/
void _cdecl drvsound_clear_queue()
{
	if	( use_sound_driver )
		audio_qhead = audio_qtail = 0;
}

void	drvsound_soundcard_driveroff()
{	use_sound_driver = false; use_sound = true;	}

void	drvsound_soundcard_driveron()
{	use_sound = use_sound_driver = true;	}

void	drvsound_nosound_at_all()
{	use_sound = false;			}



