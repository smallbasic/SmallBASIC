#ifndef w32_main
#define w32_main

#define ID_LOAD			100
#define ID_RESTART		101
#define ID_BREAK		102
#define	ID_QUIT			103
#define	ID_SBICO		400

#define	EVKEY		1
#define	EVMOUSE		2

typedef struct {
	int		type;
	char	ch;
	int		x, y;
	int		button;
	} event_t;

void	w32_evstate(int enable);
void	w32_evpush(event_t *ev);
int		w32_evget(event_t *ev);
HWND	w32_getwindow(void);
HDC		w32_getdc(void);
void	w32_copybmp(void);
#endif

