/*
*	/dev/dsp driver
*	Nicholas Christopoulos
*/

/*
*	This code is based on tonegen program.
*	ToneGen: Plays a sine wave via the dsp or standard out. 
*	ToneGen written by Timothy Pozar pozar@lns.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include <linux/soundcard.h>
#define DSPDEV "/dev/dsp"

#define TRUE		1
#define FALSE		0
#define TSIZE	4096

static	unsigned char *audio_buffer;
static	int		rate = 44100;
static	int		gotmask;
static	int		oss_h;
static	int		oss_init = 0;
static	char	device[TSIZE];
static	pthread_t	backg_tid;
volatile int audio_lock = 0;

extern unsigned char opt_quiet;

struct oss_qe_s	{
	int		freq;
	int		time;
	int		vol;
	};
typedef struct oss_qe_s oss_node;

#define	QSIZE		4096
static oss_node	audio_queue[QSIZE];
static int 		q_head, q_tail;

void*	ossdsp_backg(void*);

/*
*	open dsp
*/
int		drvsound_init()
{
	strcpy(device, DSPDEV);

	// open
	if ( (oss_h = open(device, O_WRONLY|O_NDELAY)) < 0 ) {
		if	( !opt_quiet )
	        perror("SB/OSS DRIVER");
		return 0;
		}
	else	{
		// init
		if ( ioctl(oss_h, SNDCTL_DSP_GETFMTS, &gotmask) == -1 )		{
			if	( !opt_quiet )
				perror("SB/OSS DRIVER: get dsp mask");
			return 0;
			}
		
		if ( ioctl(oss_h, SNDCTL_DSP_SPEED, &rate) == -1){
			if	( !opt_quiet )
				perror("SB/OSS DRIVER: set sample rate");
			return 0;
			}

		}

	//
	audio_buffer = (unsigned char *) malloc((rate << 1)); 
	q_head = q_tail = 0;
	oss_init = 1;
	if	( pthread_create(&backg_tid, NULL, ossdsp_backg, NULL) )	{
		if	( !opt_quiet )
			perror("SB/OSS DRIVER: background thread failed");
		oss_init = 0;
		return 0;
		}

	return 1;
}

/*
*	close
*/
void	drvsound_close()
{
	if	( oss_init )	{
		oss_init = 0;
		usleep(1000);
		close(oss_h);
		oss_h = -1;
		free(audio_buffer);
		}
}

/*
*	clear background queue
*/
void	drvsound_clear_queue()
{
	q_head = q_tail;
}

/*
*	play a tone
*/
void	ossdsp_play(int freq, int time_ms, int vol)
{
	int		count, loops;
	double	cyclesamples;
	double	f, sinnum, fu;
	short int dspnum;

	if	( !oss_init )
		return;

	if	( !freq )
		freq = rate >> 1;

	cyclesamples = ((double) rate) / ((double) freq);
	loops = (time_ms * freq/2) / 1000;

	memset(audio_buffer, 0x7f, rate << 1);

	fu = (2.0 * M_PI) / cyclesamples;
	f = fu;
	count = 0;
	while ( 1 )	{
		sinnum = sin(f);
		dspnum = 0x7fff * sinnum;

		memcpy(audio_buffer + count, &dspnum, sizeof(dspnum));
		count += sizeof(dspnum);
		write(oss_h, audio_buffer, count);
		count = 0;

		f += fu;
		if	( f > (2.0 * M_PI) )	{
			loops --;
			if	( loops <= 0 )
				return;
			f = fu;
			}
		}
}

/*
*	execute audio-list
*/
void*	ossdsp_backg(void *nil)
{
	oss_node	*node;

	while ( 1 )	{
		if ( oss_init )	{

			if	( !audio_lock )	{
				if ( q_head != q_tail )	{
					while ( q_head != q_tail )	{
						node = &audio_queue[q_head];

						ossdsp_play(node->freq, node->time, node->vol);

						q_head ++;
						if	( q_head == QSIZE )
							q_head = 0;
						}
					}
				else	
					usleep(1000);	// sleep 1ms, wait for message
				}
			else	
				usleep(100);	// sleep 0.1 ms, locked
			}
		else
			usleep(1000000);
		}

	return NULL;
}

/*
*	store sound to audio-list
*/
void	drvsound_sound(int frq, int  ms, int vol, int bgplay)
{
	oss_node	*node;

	if	( oss_init )	{
		node = &audio_queue[q_tail];
		node->freq = frq;
		node->time = ms;
		node->vol = vol;

		audio_lock = 1;
		q_tail ++;
		if	( q_tail == QSIZE )
			q_tail = 0;
		audio_lock = 0;

		if	( !bgplay )
			usleep(ms * 1000);
		}
}

/*
*	BEEP :)
*/
void	drvsound_beep()
{
	drvsound_sound(440, 250, 75, 0);
}

/*
*	not used
*/
void	drvsound_event()
{
}


