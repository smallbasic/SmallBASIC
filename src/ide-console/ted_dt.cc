/**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "dev_term.h"
#include "ted_dt.h"
#include "keys.h"

typedef struct { int code; int tedkey; } key_tra_t;

/*
*	default keyboard translation table (brief)
*/
static key_tra_t default_ktt[] =
{ 

{ SB_KEY_TAB,		TEDK_TAB },
{ SB_KEY_BACKSPACE,	TEDK_BSPACE },
{ '\e',				TEDK_ESCAPE },
{ SB_KEY_INSERT,	TEDK_PASTE	},
{ SB_KEY_DELETE,	TEDK_DELCHAR },
{ SB_KEY_F(10),		TEDK_COMMAND },
{ SB_KEY_F(3),		TEDK_SEARCH	},
{ SB_KEY_F(5),		TEDK_SEARCH },
{ SB_KEY_SF(5),		TEDK_FNEXT },
{ SB_KEY_F(1),		TEDK_HELPWORD },
{ SB_KEY_NEXT,		TEDK_NEXTSCR },
{ SB_KEY_PRIOR,		TEDK_PREVSCR },
{ SB_KEY_HOME,		TEDK_HOME },
{ SB_KEY_END,		TEDK_EOL },
{ SB_KEY_DOWN,		TEDK_DOWN },
{ SB_KEY_LEFT,		TEDK_LEFT },
{ SB_KEY_RIGHT,		TEDK_RIGHT },
{ SB_KEY_UP,		TEDK_UP },
{ '\n',				TEDK_ENTER },
{ '\r',				TEDK_ENTER },

{ SB_KEY_CTRL('L'), TEDK_REFRESH },
{ SB_KEY_CTRL('F'), TEDK_SEARCH },
{ SB_KEY_CTRL('P'), TEDK_FPREV },
{ SB_KEY_CTRL('N'), TEDK_FNEXT },
{ SB_KEY_CTRL('Z'), TEDK_UNDO },
{ SB_KEY_CTRL('Y'), TEDK_KILL },
{ SB_KEY_CTRL('A'), TEDK_INDENT },
{ SB_KEY_CTRL('C'), TEDK_COPYSEL },
{ SB_KEY_CTRL('V'), TEDK_PASTE },
{ SB_KEY_CTRL('X'), TEDK_CUTSEL },

{ SB_KEY_ALT('0'),  TEDK_MARK0 },
{ SB_KEY_ALT('1'),  TEDK_MARK1 },
{ SB_KEY_ALT('2'),  TEDK_MARK2 },
{ SB_KEY_ALT('3'),  TEDK_MARK3 },
{ SB_KEY_ALT('4'),  TEDK_MARK4 },
{ SB_KEY_ALT('5'),  TEDK_MARK5 },
{ SB_KEY_ALT('6'),  TEDK_MARK6 },
{ SB_KEY_ALT('7'),  TEDK_MARK7 },
{ SB_KEY_ALT('8'),  TEDK_MARK8 },
{ SB_KEY_ALT('9'),  TEDK_MARK9 },

{ SB_KEY_ALT('b'),  TEDK_BUFLIST },
{ SB_KEY_ALT('j'),  TEDK_GMARK },
{ SB_KEY_ALT('i'),  TEDK_INSOVR },
{ SB_KEY_ALT('g'),  TEDK_GOTO },
{ SB_KEY_ALT('w'),  TEDK_SAVE },
{ SB_KEY_ALT('e'),  TEDK_LOAD },
{ SB_KEY_ALT('o'),  TEDK_CHANGEFN },
{ SB_KEY_ALT('x'),  TEDK_SYSEXIT },
{ SB_KEY_ALT('k'),  TEDK_DELEOL },	// delete to end-of-lin },
{ SB_KEY_ALT('m'),  TEDK_SELMARK },
{ SB_KEY_ALT('l'),  TEDK_LINESELMARK },
{ SB_KEY_ALT('d'),  TEDK_KILL },
{ SB_KEY_ALT('h'),  TEDK_HELP },
{ SB_KEY_ALT('u'),  TEDK_UNDO },
{ SB_KEY_ALT('p'),  TEDK_PREVBUF },
{ SB_KEY_ALT('n'),  TEDK_NEXTBUF },

{ SB_KEY_ALT('*'),  TEDK_UNDO },
{ SB_KEY_ALT('-'),  TEDK_CUTSEL },
{ SB_KEY_ALT('+'),  TEDK_COPYSEL },

{ SB_KEY_KP_DIV,    '/' },
{ SB_KEY_KP_MUL,    TEDK_UNDO },
{ SB_KEY_KP_MINUS,  TEDK_CUTSEL },
{ SB_KEY_KP_PLUS,   TEDK_COPYSEL },
{ SB_KEY_KP_MUL,    TEDK_UNDO },
{ SB_KEY_KP_ENTER,  '\n' },
{ SB_KEY_KP_HOME,	TEDK_HOME },
{ SB_KEY_KP_UP,		TEDK_UP },
{ SB_KEY_KP_PGUP,	TEDK_PREVSCR },
{ SB_KEY_KP_LEFT,	TEDK_LEFT },
{ SB_KEY_KP_CENTER,	TEDK_CENTER },
{ SB_KEY_KP_RIGHT,	TEDK_RIGHT },
{ SB_KEY_KP_END,	TEDK_EOL },
{ SB_KEY_KP_DOWN,	TEDK_DOWN },
{ SB_KEY_KP_PGDN,	TEDK_NEXTSCR },

{ SB_KEY_KP_INS,	TEDK_PASTE },
{ SB_KEY_KP_DEL,	TEDK_DELCHAR },

{ 0, 0 }
};

/// USER DEFINED KEYS ////////////////////////////////////////////////////////

/*
*	user keyboard translation table
*/
static key_tra_t user_ktt[512];
static int		 user_ktt_count;

/*
*	key names and description
*/
typedef struct { int tedkey; char name[64]; char desc[256]; } key_name_t;

static key_name_t key_name[] = {
{	TEDK_TAB,			"tab",			"tab"	},
{	TEDK_ESCAPE,		"escape",		"escape"	},
{	TEDK_ENTER,			"enter",		"enter"		},
{	TEDK_DELETE,		"deletekey",	"delete key"	},
{	TEDK_BSPACE,		"delback",		"delete one character backward"	},
{	TEDK_INSOVR,		"insert_sw",	"insert/overwrite switch"	},
{	TEDK_INDENT,  		"ident_sw",		"auto-ident switch"	},
{	TEDK_SENSIT,  		"case_sw",		"case sensitivity switch" },
{	TEDK_CHCASE,		"chcase",		"change case"	},
{	TEDK_MATCH,			"match",		"N/A: match"	},
{	TEDK_DELCHAR,		"delforw",		"delete one character forward"	},
{	TEDK_DELSEL,		"delsel",		"delete selected text"	},
{	TEDK_COPYSEL,		"copy",			"copy selected text"	},
{	TEDK_PASTE,			"paste",		"paste cliboard text"	},
{	TEDK_MARK0,			"mark0",		"setup bookmark 0"	},
{	TEDK_MARK1,			"mark1",		"setup bookmark 1"	},
{	TEDK_MARK2,			"mark2",		"setup bookmark 2"	},
{	TEDK_MARK3,			"mark3",		"setup bookmark 3"	},
{	TEDK_MARK4,			"mark4",		"setup bookmark 4"	},
{	TEDK_MARK5,			"mark5",		"setup bookmark 5"	},
{	TEDK_MARK6,			"mark6",		"setup bookmark 6"	},
{	TEDK_MARK7,			"mark7",		"setup bookmark 7"	},
{	TEDK_MARK8,			"mark8",		"setup bookmark 8"	},
{	TEDK_MARK9,			"mark9",		"setup bookmark 9"	},
{	TEDK_GMARK,			"goto_mark",	"go to bookmark"	},
{	TEDK_GOTO,			"goto_line",	"go to line"		},
{	TEDK_SEARCH,		"search",		"search for ..."	},
{	TEDK_FPREV,			"fprev",		"find previous"		},
{	TEDK_FNEXT,			"fnext",		"find next"			},
{	TEDK_REPEAT,		"repeat",		"N/A: repeat"		},
{	TEDK_SUBST,			"subst",		"N/A: substitute"	},
{	TEDK_REFRESH,		"refresh",		"refresh screen"	},
{	TEDK_PREVSCR,		"page_up",		"page up"			},
{	TEDK_BOF,			"bof",			"go to begin-of-file" },
{	TEDK_NEXTSCR,		"page_down",	"page down"			},
{	TEDK_EOF,			"eof",			"go to end-of-file"	},
{	TEDK_KILL, 			"kill",			"delete line"		},
{	TEDK_WORD,			"wforw",		"go to one word forward" },
{	TEDK_DELWORD, 		"delnextword",	"delete next word" },
{	TEDK_BWORD,			"wback",		"go to one word backward" },
{	TEDK_DELBWORD,		"delbackword",	"delete back word" },
{	TEDK_UNDO,			"undo",			"undo"	},
{	TEDK_HOME,			"home",			"go to begin-of-line" },
{	TEDK_DELBOL,		"delbol",		"delete to begin of line" },
{	TEDK_EOL,			"end",			"go to end-of-line" },
{	TEDK_DELEOL,		"deleol",		"delete to end-of-line" },
{	TEDK_AGAIN,			"repagain",		"N/A: repeat again" },
{	TEDK_DOWN,			"down",			"down" },
{	TEDK_LEFT,			"left",			"left"	},
{	TEDK_RIGHT,			"right",		"right"	},
{	TEDK_UP,			"up",			"up" },
{	TEDK_MENU,			"menu",			"N/A: menu" },
{	TEDK_COMMAND, 		"command",		"N/A: parse command" },
{	TEDK_HELP,			"help",			"help screen" },
{	TEDK_CENTER,		"center",		"screen center" },
{	TEDK_ALIAS,			"alias",		"N/A: alias"	},
{	TEDK_HELPWORD,		"wordhelp",		"on-line help (search for word)" },
{	TEDK_SELMARK, 		"selstart",		"selection start" },
{	TEDK_LINESELMARK,	"lselstart",	"line-selection start" },
{	TEDK_SELCANCEL,		"selstop",		"cancel selection" },
{	TEDK_CUTSEL,		"cut",			"cut" },
{	TEDK_LINECUT,  		"lcut",			"line-cut" },
{	TEDK_LINECOPY,		"lcopy",		"line-copy" },
{	TEDK_BUFLIST,		"buflist",		"list of buffers" },
{	TEDK_SYSEXIT,		"quit",			"quit" },
{	TEDK_SAVE,			"save",			"save file" },
{	TEDK_LOAD,			"load",			"load file" },
{	TEDK_CHANGEFN,		"changefn",		"change file-name" },
{	TEDK_NEXTBUF, 		"nextbuf",		"go to next buffer" },
{	TEDK_PREVBUF,		"prevbuf",		"go to previous buffer" },
{	TEDK_NULL,			"",				""	}
};

#define		TERM_WAIT	(1000000 / 1920)

/*
*	returns the key-code (TEDK) of the key with 'name'
*/
int		find_key_by_name(const char *name)
{
	int		i;

	for ( i = 0; key_name[i].tedkey != TEDK_NULL; i ++ )	{
		if	( strcmp(name, key_name[i].name) == 0 )
			return key_name[i].tedkey;
		}
	return 0;
}

/*
*	returns the key name (job)
*/
const char *find_name_by_code(int tedkey)
{
	int		i;

	for ( i = 0; key_name[i].tedkey != TEDK_NULL; i ++ )	{
		if	( key_name[i].tedkey == tedkey )
			return key_name[i].name;
		}
	return NULL;
}

/*
*	returns the description of the key 
*/
const char *find_desc_by_code(int tedkey)
{
	int		i;

	for ( i = 0; key_name[i].tedkey != TEDK_NULL; i ++ )	{
		if	( key_name[i].tedkey == tedkey )
			return key_name[i].desc;
		}
	return NULL;
}

/*
*	loads a key table
*/
void	load_key_table(const char *filename)
{
	FILE	*fp;
	int		sbk, tedk;
	char	buf[128], *p, *ps;

	fp = fopen(filename, "rt");
	if	( fp )	{
		while ( fgets(buf, 128, fp) )	{
			// remarks
			p = strchr(buf, '#');
			if	( p )	*p = '\0';

			// separator
			p = strchr(buf, ' ');
			if	( p )	{
				*p = '\0';	p ++;
				while ( *p == ' ' || *p == '\t' )		p ++;
				ps = p;	// ps points to key-name
				while ( *p >= '0' && *p <= 'z' )		p ++;
				*p = '\0';	// trim key-name

				// store key
				user_ktt[user_ktt_count].code   = atoi(buf);
				user_ktt[user_ktt_count].tedkey = find_key_by_name(ps);
				user_ktt_count ++;
				}
			}
		fclose(fp);
		}
}

/*
*	saves a key table
*/
void	save_key_table(const char *filename)
{
	FILE	*fp;
	int		sbk, tedk;
	char	buf[128], *p;
	int		i;

	fp = fopen(filename, "wt");
	if	( fp )	{
		for ( i = 0; i < user_ktt_count; i ++ )	{
			fprintf(fp, "%d %s # %s\n",
				user_ktt[i].code,
				find_name_by_code(user_ktt[i].tedkey),
				find_desc_by_code(user_ktt[i].tedkey) );
			}
		fclose(fp);
		}
}

/////////////////////////////////////////////////////////////////////////

//
TedDT::TedDT() : TED()
{
	term_cls();
	repaint = false;
	finished = false;
}

//
TedDT::~TedDT()
{
}

// clear screen
void	TedDT::clrscr()
{
	term_cls();
}

// clear to EOL
void	TedDT::clreol()
{
	term_print("\033[K");
}

// set cursor position
void	TedDT::gotoxy(int x, int y)
{
	term_setxy(x, y);
}

// get cursor position
void	TedDT::getxy(int *x, int *y)
{
	*x = term_getx();
	*y = term_gety();
}

// delete the line at cursor and scroll up the rests
void	TedDT::deleteln()
{
	repaint = true;
}

// insert an empty line at cursor and scroll down the rests
void	TedDT::insertln()
{
	repaint = true;
}

// printf    
void	TedDT::printw(const char *fmt, ...)
{
	char	buf[1024];
	va_list argptr;

	va_start(argptr, fmt);
	vsprintf(buf, fmt, argptr);
	va_end(argptr);
	term_print(buf);
}

// gets a key (similar to getch() but for extended keys uses > 0x100)
int		TedDT::get_pref_ch()
{
	int		c;

	term_setcursor(1);
	fflush(stdout);
	c = translate(term_getch());
	term_setcursor(0);
	fflush(stdout);
	return c;
}

// gets a key (similar to getch() but for extended keys uses > 0x100)
int		TedDT::get_row_ch()
{
	int		c;

	term_setcursor(1);
	fflush(stdout);
	c = term_getch();
	term_setcursor(0);
	fflush(stdout);
	return c;
}

// returns the number of the rows of the window
int		TedDT::scr_rows()
{
    return term_rows() - 1;
}

// returns the number of the columns of the window
int		TedDT::scr_cols()
{
	return term_cols();
}

// just produce a beep
void	TedDT::beep()
{
	printf("\a");
}

// print a char at current position
void	TedDT::addch(int ch)
{
	printw("%c", ch);
}

// user request: exit (Alt+X)
void	TedDT::sys_exit()
{
	finished = true;
}

// translate real key-code to TEDK_xxx code
int		TedDT::translate(int c)
{
	int		i;

	// user-defined keys
	for ( i = user_ktt_count - 1; i >= 0; i -- )	{
		if	( user_ktt[i].code == c )
			return user_ktt[i].tedkey;
		}

	// brief's keys
	for ( i = 0; default_ktt[i].code; i ++ )	{
		if	( default_ktt[i].code == c )
			return default_ktt[i].tedkey;
		}

	// unknown key
	if	( c & 0xFF00 )	{	// extented key
		prompt("Unknown key: 0x%04X", c);
		return 0;
		}

	//
	return c;
}

// execute key
int		TedDT::key(int ch)
{
	manage_key(ch);
	place_cursor();
} 

//
void	TedDT::update_cursor()
{
}

/**
*/
void	TedDT::paint()
{
	adjust_screen(1);
}

/**
*	flush screen
*/
void	TedDT::realrefresh()
{
	if	( lock_refresh )	return;
	lock_refresh = true;

	if	( repaint )	{
		redraw_screen(false);
		repaint = false;
		}
	place_cursor();
	fflush(stdout);
	lock_refresh = false;
}

/**
*	flush screen
*/
void	TedDT::refresh()
{
	realrefresh();
}

/**
*	setup the font
*/
void	TedDT::set_font(const char *font, int size)
{
}

/**
*	emulation of VGA16 text colors
*/
void	TedDT::set_color(int color)
{
	term_settextcolor(color & 0xF, (color & 0xF0) >> 4);
}

/**
*	editor's notification: copy to clipboard
*/
void	TedDT::clip_copy()
{
}

/**
*	editor's notification: paste from clipboard
*/
void	TedDT::clip_paste()
{
}

/*
*/
void	TedDT::help_on_word(const char *word)
{
#if defined(_SBIDE)
	extern char	*ide_exec(const char *,bool);

	char	cmd[1024];
	char	*out;
	int		warea=3;	// workpad
	
	sprintf(cmd, "sbasic -h-%s", word);
	out = ide_exec(cmd, false);
	if	( !out )	{
		prompt("SmallBASIC compiler not found. Please reinstall SmallBASIC");
		beep();
		}
	else	{
		set_text(out, warea);
		modified(false, warea);
		activate(warea);
		free(out);
		}
#endif
}
