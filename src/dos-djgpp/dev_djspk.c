/*
*	DJGPP's speaker
*
*	This is an excellent example of add-on sound driver
*	Nicholas Christopoulos
*/

#include "sys.h"
#include "drvsound.h"
#include <time.h>
#include <pc.h>

#define	 TPS	CLOCKS_PER_SEC

/*
*	background sound queue
*/
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

/*
*	initialization
*/
int		drvsound_init()
{
	nosound();
	audio_queue = dbt_create("vAUDIO", 0);
	dbt_prealloc(audio_queue, AUDIO_QSIZE, sizeof(audio_node));
	audio_qhead = audio_qtail = 0;
	return 1;
}

/*
*	close the driver
*/
void	drvsound_close()
{
	audio_qhead = audio_qtail = 0;
	dbt_close(audio_queue);
	nosound();
}

/*
*	the real play tone
*	(ms is unused)
*/
void	drvsound_realsound(int frq, int ms, int vol)
{
	if	( frq )
		sound(frq);
	else
		nosound();
}

/*
*	background play loop
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

/*
*	SB: play sound
*/
void	drvsound_sound(int frq, int ms, int vol, int bgplay)
{
	if	( !bgplay )	{
		drvsound_realsound(frq, ms, vol);
		delay(ms);
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

/*
*	SB: system beep
*/
void	drvsound_beep()
{
	drvsound_sound(440, 125, 75, 0);
}

/*
*	SB system wants to check our queue
*/
void	drvsound_event()
{
	drvsound_backsound(0);
}

