/*
*/

#if !defined(__bcb_h)
#define __bcb_h

#if defined(__cplusplus)
extern "C" {
#endif

#include <windows.h>
#undef V_INT
#undef V_ARRAY

void	bcb_mgrerr(char *fmt, ...);
void	bcb_doevents(int wait_flag);

/*
*	console
*/
void	bcb_cls(void);
void	bcb_gets(char *dest, int size);
void	bcb_write(char *str);

/*
*	graphics
*/
void	bcb_line(int x1, int y1, int x2, int y2, int color);
void	bcb_rect(RECT r, int color, int fill);

#if defined(__cplusplus)
}
#endif
 
#endif