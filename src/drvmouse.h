/*
*	Add-on SB mouse driver header
*	Nicholas Christopoulos
*/

#if !defined(_DRV_MOUSE_H)
#define _DRV_MOUSE_H

/*
*   driver initialization
*   returns false on error
*/
int		drvmouse_init(void);

/*
*   restores device state
*/
void	drvmouse_close(void);

/*
*   returns true if there is a new event on device
*
*   if device has event 
*      x,y      = position of mouse cursor
*      buttons  = buttons state (bits 1,2,4,8 for 4-button mouse,
*                   starting from 1 (if only the left-button is pressed; *buttons=1)
*/
int		drvmouse_get(int *x, int *y, int *buttons);

#endif

