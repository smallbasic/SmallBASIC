#include <windows.h>
#include "sys.h"
#include "w32_main.h"
#include "device.h"

volatile HWND cur_wnd;
int	realmain(int argc, char *argv[]);	// brun
typedef char* char_p;
void	w32_stdout_open(void);
void	w32_stdout_close(void);
extern byte	opt_graphics;

/* ------------------------------------------------------------------------------------------------------------------- */

/*
*	events queue
*/

#define	EVLIST_SIZE		1024
volatile event_t	evlist[EVLIST_SIZE];
volatile int		ev_head, ev_tail;
volatile int		ev_record;
volatile HDC		mem_hdc;
volatile HBITMAP	mem_bmp, mem_oldbmp;
static	 int		width = 100, height = 100;
static	 HINSTANCE	g_hInstance;
volatile int		stdout_init = 0;
static int			g_argc;
static char_p		*g_argv;
static char			wcmd[1024];
static char			srcfname[1024];
static char			sb4w32_inifile[1024];
volatile int		running = 0;
static char	*appname = "SB4W32";


/*
*	start/stop storing events
*/
void	w32_evstate(int enable)
{
	ev_head = ev_tail = 0;
	ev_record = enable;
}

/*
*	stores an event to queue
*/
void	w32_evpush(event_t *ev)
{
	if	( ev_record )	{
		evlist[ev_tail] = *ev;
		if	( ev_tail+1 >= EVLIST_SIZE )
			ev_tail = 0;
		else
			ev_tail ++;
		}
}

/*
*	returns true if there is an event in queue (event is COPIED in the 'ev')
*/
int		w32_evget(event_t *ev)
{
	if ( ev_tail == ev_head )
		return 0;

	*ev = evlist[ev_head];
	if	( ev_head+1 >= EVLIST_SIZE )
		ev_head = 0;
	else
		ev_head ++;
	return 1;
}

/*
*/
HWND	w32_getwindow()
{
	return cur_wnd;
}

/*
*/
HDC		w32_getdc()
{
	return mem_hdc;
}

/*
*/
void	w32_refresh()
{
	RECT	r;

	GetClientRect(cur_wnd, &r);
	InvalidateRect(cur_wnd, &r, 0);
}

/*
*/
void	w32_copybmp()
{
	HDC				realhdc;
	PAINTSTRUCT     ps;

	realhdc = BeginPaint(cur_wnd, &ps);
	if	( stdout_init )	{
		BitBlt(
			realhdc, 0, 0, width, height,
			mem_hdc, 0, 0, SRCCOPY);
		}
	else	{
		RECT	r;

		GetClientRect(cur_wnd, &r);
		FillRect(realhdc, &r, (HBRUSH) (COLOR_WINDOW+1));
		}
	EndPaint(cur_wnd, &ps);
}

/* ------------------------------------------------------------------------------------------------------------------- */

void	sbr_shell(void *unused)
{
	running = 1;

//	realmain(g_argc, g_argv);
	memmgr_init();
	brun(srcfname, 0);
	memmgr_close();

	w32_refresh();

	running = 0;
}

void	build_cmds(void)
{
	char	*p, *ps;
	int		count = 0, q = 0, i;
	char	*pt[128];

	ps = p = wcmd;
	while ( *p )	{
		if	( *p == '\"' )	{
			q = !q;
			*p = ' ';
			}
		else if	( !q && (*p == ' ' || *p == '\t') )	{
			// end of arg
			*p = '\0';	p ++;
			while ( *ps == ' ' || *ps == '\t' )		ps ++;
			if	( *ps )	{
				pt[count] = ps;
				count ++;
				}
			ps = p;
			}

		if	( *p )
			p ++;
		}

	if	( *ps )	{
		pt[count] = ps;
		count ++;
		}

	count ++;	// argv[0]

	g_argv = malloc(sizeof(char_p) * count);
	g_argc = count;
	for ( i = 0; i < count-1; i ++ )	
		g_argv[i+1] = pt[i];

	g_argv[0] = (char *) malloc(1024);
	GetModuleFileName(NULL, g_argv[0], 1024);

	// ini
	strcpy(sb4w32_inifile, g_argv[0]);
	p = strrchr(sb4w32_inifile, '.');
	*p = '\0';
	strcat(sb4w32_inifile, ".ini");
}

/*
*/
void	scan_cmds()
{
	int		opt_decomp = 0;
	char	cwd[1025], *slash;
	int		i;

	dev_printf("SmallBASIC version %s\n", SB_STR_VER);
	dev_printf("Written by Nicholas Christopoulos\n");
	dev_printf("http://smallbasic.sourceforge.net\n");
	strcpy(srcfname, "");
	opt_graphics = 2;
	for ( i = 1; i < g_argc; i ++ )	{
		if	( g_argv[i][0] == '-' )	{
			if	( g_argv[i][1] == 's' )
				opt_decomp ++;
			if	( g_argv[i][1] == 'g' )
				opt_graphics = 2;
			if	( g_argv[i][1] == 'j' )
				os_charset = enc_sjis;
			if	( g_argv[i][1] == 'b' )
				os_charset = enc_big5;
			if	( g_argv[i][1] == 'm' )
				os_charset = enc_gmb;
			if	( g_argv[i][1] == 'u' )
				os_charset = enc_unicode;
			if	( g_argv[i][1] == 'x' )	{
				slash = &g_argv[i][2];
				sprintf(cwd, "SBGRAF=%s", slash);
				putenv(cwd);
				}
			if	( g_argv[i][1] == 'h' )	{
				dev_printf("usage: sbasic source [options]\n");
				dev_printf("-s		decompiler\n");
				dev_printf("-g		enable graphics (ignored; its always on)\n");
				dev_printf("-x<width>x<height>[x<bpp>] graphics mode (only for DOSGRX, XF and Win32 driver)\n");
/*
				fprintf(stderr, "\ncharset (default: utf8)\n");
				fprintf(stderr, "-j		enable sjis\n");
				fprintf(stderr, "-b		enable big5\n");
				fprintf(stderr, "-m		enable generic multibyte\n");
				fprintf(stderr, "-u		enable unicode!\n");
*/
				}
			}
		else
			strcpy(srcfname, g_argv[i]);
		}
}

/* ------------------------------------------------------------------------------------------------------------------- */

/*
*/
int		loadfile()
{
	OPENFILENAME    op;
	char			*filter = "BASIC files (*.bas)\0*.bas\0\0";
	char			file[1024], dir[1024], *p;
					
	GetPrivateProfileString(appname, "LastDir", "", dir, 1024, sb4w32_inifile);
	file[0] = '\0';
	op.lStructSize = sizeof(OPENFILENAME);
	op.hwndOwner = cur_wnd;
	op.hInstance = g_hInstance;
	op.lpstrFilter = filter;
	op.lpstrCustomFilter = NULL;
	op.nFilterIndex = 1;
	op.lpstrFile = file;
	op.nMaxFile = 1024;
	op.lpstrFileTitle = NULL;
	op.nMaxFileTitle = 0;
	if	( strlen(dir) )
		op.lpstrInitialDir = dir;
	else
		op.lpstrInitialDir = NULL;
	op.lpstrTitle = "Load file";
	op.Flags = OFN_HIDEREADONLY;
	op.nFileOffset = 0;
	op.nFileExtension = 0;
	op.lpstrDefExt = "bas";

	if ( GetOpenFileName(&op) )		{
		strcpy(srcfname, op.lpstrFile);
		strcpy(dir, srcfname);
		p = strrchr(dir, '\\');
		if	( p )	
			WritePrivateProfileString(appname, "LastDir", dir, sb4w32_inifile);
		return 1;
		}
	return 0;
}

/* ------------------------------------------------------------------------------------------------------------------- */

void	close_dev()
{
	if	( stdout_init )	{
		w32_stdout_close();
		// destroy DC
		SelectObject(mem_hdc, mem_oldbmp);
		DeleteDC(mem_hdc);
		DeleteObject(mem_bmp);
		stdout_init = 0;
		}
}

void	reset_dev()
{
	HDC		realhdc;
	RECT	r;

	if	( stdout_init )
		close_dev();

	// build DC
	if	( !stdout_init )	{
		GetClientRect(cur_wnd, &r);
		realhdc = GetDC(cur_wnd);
		mem_hdc = CreateCompatibleDC(realhdc);
		width   = (r.right-r.left) +1;
		height  = (r.bottom-r.top) +1;
		mem_bmp = CreateCompatibleBitmap(realhdc, width, height+8);
		mem_oldbmp = (HBITMAP) SelectObject(mem_hdc, mem_bmp);
		FillRect(mem_hdc, &r, (HBRUSH) (COLOR_WINDOW+1));
		ReleaseDC(cur_wnd, realhdc);

		w32_stdout_open();
		stdout_init = 1;
		}
}

/*
*	Send BREAK key to executor
*/
void	sbbreak()
{
	event_t	ev;

	while ( running )	{
		// send CTRL+C
		ev.type = EVKEY;
		ev.ch = 3;
		w32_evpush(&ev);
		Sleep(1000);
		}
}

/*
*	run (or restart) a program
*/
void	sbrun()
{
	if	( stdout_init )
		sbbreak();

	reset_dev();

	if	( strlen(srcfname) )	{
		char	buf[1024];
		char	*p;

		strcpy(buf, "SmallBASIC for Win32 - ");
		p = strrchr(srcfname, '\\');
		if	( p )
			strcat(buf, p+1);
		else	{
			p = strrchr(srcfname, '/');
			if	( p )
				strcat(buf, p+1);
			else	
				strcat(buf, srcfname);
			}
		SetWindowText(cur_wnd, buf);
		_beginthread(sbr_shell, 0, NULL);		// run it in background thread
		}
}

/*
*	standard window procs
*/
long FAR PASCAL w32WndProc(HWND hWnd, unsigned message, WPARAM wParam, LPARAM lParam)
{
	event_t	ev;
	POINT	pt;
	RECT	r;

	switch (message)	{              
	case WM_CREATE:
		cur_wnd = hWnd;
		build_cmds();							// convert command-line string to argc/argv
		scan_cmds();

		sbrun();
		break;
	    
	case WM_COMMAND:             // Get messages from menu bar
		switch(wParam)		{
		case ID_LOAD:
			if	( loadfile() )	
				sbrun();
			break;
		case ID_RESTART:
			sbrun();
			break;
		case ID_BREAK:
			sbbreak();
			break;
		case ID_QUIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
			}
		break; 
	case WM_PAINT:
		w32_copybmp();
		break;

	case WM_LBUTTONDOWN:        case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:        case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:        case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
		ev.type = EVMOUSE;
		ev.x = LOWORD(lParam);
		ev.y = HIWORD(lParam);
		ev.button = 
			((wParam & MK_MBUTTON) ? 0x4 : 0 ) |
			((wParam & MK_RBUTTON) ? 0x2 : 0 ) |
			((wParam & MK_LBUTTON) ? 0x1 : 0 );
		w32_evpush(&ev);
		break;

	case WM_CHAR:
		ev.type = EVKEY;
		ev.ch = wParam;
		w32_evpush(&ev);
		break;
		    
	case WM_DESTROY:             // Destroy window
		sbbreak();

		close_dev();
		
		// quit
		PostQuitMessage(0);
		break;
	
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		}

	return(0L);
}

BOOL	w32init(HANDLE hInstance)
{
	WNDCLASS        wcMenuClass;

	wcMenuClass.hCursor         =   LoadCursor(NULL, IDC_ARROW);
	wcMenuClass.hIcon           =   LoadIcon(NULL, MAKEINTRESOURCE(ID_SBICO));
	wcMenuClass.lpszMenuName    =   (LPSTR)"GENERIC";
	wcMenuClass.lpszClassName   =   (LPSTR)appname;
	wcMenuClass.hbrBackground   =   (HBRUSH)(COLOR_WINDOW+1);
	wcMenuClass.hInstance       =   hInstance;
	wcMenuClass.style           =   CS_VREDRAW | CS_HREDRAW;
	wcMenuClass.lpfnWndProc     =   w32WndProc;
	wcMenuClass.cbClsExtra      =   0 ;
	wcMenuClass.cbWndExtra      =   0 ;

	if ( !RegisterClass(&wcMenuClass) )
		return FALSE;
	return TRUE;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int cmdShow)
{
	MSG     msg;                         // Message variable
	HWND    hWnd;                        // Window handle
	int		x1, y1, x2, y2;
	DWORD	style, exstyle;

	if ( !hPrevInstance ) {
		if ( !w32init(hInstance) )
			return FALSE;
		}
	
	if	( lpszCmdLine )
		strcpy(wcmd, lpszCmdLine);
	else
		strcpy(wcmd, "");

	g_hInstance = hInstance;

	// main window style
	style = WS_OVERLAPPEDWINDOW;
	exstyle = WS_EX_CLIENTEDGE | WS_EX_TOOLWINDOW /* | WS_EX_TOPMOST */;

	x1 = y1 = 100;
	x2 = x1 + 400;
	y2 = y1 + 300;

	// create the main window
	stdout_init = 0;
	running = 0;
	strcpy(srcfname, "");

	hWnd = CreateWindowEx (exstyle, appname, "SmallBASIC for Win32", style,
                        x1, y1, x2, y2, NULL, NULL, hInstance, NULL);
	cur_wnd = hWnd;

	ShowWindow(hWnd, cmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg,NULL,0,0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		}
	return (int)msg.wParam;
}
