/*
*	mouse driver
*	This driver supports only the gpm
*
*	Nicholas Christopoulos
*/

/*
*	This code is based on OFBIS library's code
*	OFBIS is a framebuffer graphics library by Tomas Berndtsson
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

static int 	msefd;
static int	devtype = 0;

/*
*/
int		drvmouse_init()
{
	if	( access("/dev/gpmctl", F_OK) != 0 )	{
		fprintf(stderr, "SB/GPM(MOUSE) DRIVER: error opening /dev/gpmdata\n");
		return 0;
		}

	devtype = 1;
	if ( (msefd = open("/dev/gpmdata", O_RDWR | O_NDELAY)) == -1 ) {
	    if ( (msefd = open("/dev/mouse", O_RDWR | O_NDELAY)) == -1 ) {	// SuSE 7.2 default
    		devtype = 0;
			fprintf(stderr, "SB/GPM(MOUSE) DRIVER: error opening /dev/gpmdata\n");
			return 0;
			}
		}

	tcflush(msefd, TCIFLUSH);
	return 1;
}

/*
*/
void	drvmouse_close()
{
	close(msefd);
}

/*
*	returns true on event
*/
int		drvmouse_get(int *x, int *y, int *buttons)
{
	char    buf[6];

	if ( read(msefd, &buf, 6) == -1 )	
		return 0;

	buf[1] = buf[3]*2;
	buf[2] = buf[4]*2;

	*buttons = buf[0] & 0x07;
	*x = (int) (   (signed char) (buf[1] & 0xFF));
	*y = (int) ( - (signed char) (buf[2] & 0xFF));
	return 1;
}


