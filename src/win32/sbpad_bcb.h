#if !defined(_w32_sbpad_h)
#define _w32_sbpad_h

#define	EVKEY		1
#define	EVMOUSE		2

typedef struct {
	int		type;
	int		ch;
	int		x, y;
	int		button;
	} event_t;

void _cdecl	w32_evstate(int enable);
void _cdecl	w32_evpush(event_t *ev);
int  _cdecl	w32_evget(event_t *ev);

TCanvas* _cdecl	w32_canvas(void);
void _cdecl w32_invalidate_rect(void);

#endif

