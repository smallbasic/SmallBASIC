/*
 * mouse driver
 * This driver supports only the gpm
 *
 * Nicholas Christopoulos
 */

/*
 * This code is based on OFBIS library's code
 * OFBIS is a framebuffer graphics library by Tomas Berndtsson
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <gpm.h>

static Gpm_Connect conn;
static int use_mouse = 0;

/*
 */
int drvmouse_init()
{
  use_mouse = 0;

  conn.eventMask = 0;
  conn.defaultMask = GPM_MOVE | GPM_HARD;
  conn.maxMod = 0;
  conn.minMod = 0;

  if (Gpm_Open(&conn, 0) < 0) { // -1 = console error, -2 = xterm (stdin must
    // be used)
    fprintf(stderr, "Can't open mouse connection\n");
    return 0;
  }

  return (use_mouse = 1);
}

/*
 */
void drvmouse_close()
{
  if (use_mouse)
    Gpm_Close();
}

/*
 * returns true on event
 */
int drvmouse_get(int *x, int *y, int *buttons)
{
  if (use_mouse) {
    Gpm_Event evt;

    if (Gpm_GetEvent(&evt) > 0) {
      *x = evt.x;
      *y = evt.y;
      *buttons = evt.buttons;
    }
  }

  return 1;
}
