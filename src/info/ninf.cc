/**
*	Info, man-page and text-file viewer
*
*	2002-10-12, Nicholas Christopoulos
*
*	Linux, xterm and ANSI terminals are supported
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>		// struct timeval
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <term.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "info.h"

/* the info object */
static Info	inf;

/*
*	the colors
*/
//static const char *clr_reset  = "\033[0m\033[40m\033[5m";
static const char *clr_reset  = "\033[0m\033[44m\033[37m";			// reset terminal to defaults
static const char *clr_menu   = "\033[1m\033[36m";					// menu-item 
static const char *clr_mnusel = "\033[30m\033[46m";					// selected menu-item
static const char *clr_italics= "\033[1m\033[37m";					// italics
static const char *clr_bold   = "\033[1m\033[33m";					// bold
static const char *clr_header = "\033[37m";							// header
static const char *clr_footer = "\033[37m";							// footer
static const char *clr_lines  = "\033[1m\033[34m";					// other info (line numbers, eof/bof)
static const char *clr_hdrhi  = "\033[1m\033[36m";					// highligheted header/footer items

static const int  page_margin = 8;									// default left page margin
static const int  top_margin = 1;									// default top page margin

static char cur_node_name[256];										// the name of the current node
static char last_search_string[256];								// the last search string
static int  cur_page_top, max_page_size;							// size and y-offset of the current page

/* --- TERMINAL I/O -------------------------------------------------------------------------------------- */

static int screen_width = 80;
static int screen_height = 25;

/* xterm resize */
#if defined(_UnixOS)
void	cons_sig_winch(int a)
{
#if defined(TIOCGWINSZ)
   	struct winsize ws;

   	/* get and check window size. */
	if ( ioctl(0, TIOCGWINSZ, &ws) >= 0 && ws.ws_row != 0 && ws.ws_col != 0 )  {
       	screen_width = ws.ws_col;
       	screen_height = ws.ws_row;
       	}
#endif
}
#endif

/* initialize console */
void	cons_init()
{
#if defined(TIOCGWINSZ)
   	struct winsize ws;

	signal(SIGWINCH, cons_sig_winch);

   	/* get and check window size. */
	if ( ioctl(0, TIOCGWINSZ, &ws) >= 0 && ws.ws_row != 0 && ws.ws_col != 0 )  {
       	screen_width = ws.ws_col;
       	screen_height = ws.ws_row;
       	}
#else
	screen_width = 80;
	screen_height = 25;	
#endif
}

/*	print right-adjusted text */
void	cons_rprintf(const char *fmt, ...)
{
	va_list	ap;
	char	msg[2048];

	va_start(ap, fmt);
	vsnprintf(msg, 2048, fmt, ap);
	va_end(ap);

	printf("\033[%dG%s", (screen_width - strlen(msg)) + 1, msg);
}

/*	print on the line */
void	cons_lprintf(const char *fmt, ...)
{
	va_list	ap;
	char	msg[2048];
	int		i;

	va_start(ap, fmt);
	vsnprintf(msg, 2048, fmt, ap);
	va_end(ap);

	for ( i = 0; i < screen_width; i ++ )		printf("_");
	printf("\033[3G %s \n", msg);
}

/* prints at */
void	cons_pratf(int x, int y, const char *fmt, ...)
{
	va_list	ap;
	char	msg[2048];

	printf("\033[%d;%dH", y, x);
	va_start(ap, fmt);
	vsnprintf(msg, 2048, fmt, ap);
	va_end(ap);
	printf("%s", msg);
}

/* return keycode */
int		cons_getkey(int waitf)
{
	fd_set		rfds;
	struct timeval tv;
	int			ival;
	int			c = 0;
	struct termios saved_attributes;
	struct termios tattr;

	// save attributes
	tcgetattr (STDIN_FILENO, &saved_attributes);
	fflush(stdout);

	//
	tcgetattr (STDIN_FILENO, &tattr);
	tattr.c_lflag &= ~(ICANON|ECHO|ECHONL); // Clear ICANON and ECHO.
	tattr.c_cc[VMIN] = 1;
	tattr.c_cc[VTIME] = 0;
	tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);

	//
	do	{
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		tv.tv_sec=0;
		tv.tv_usec=500;
		ival = select(1, &rfds, NULL, NULL, &tv);
		if	( ival )	{
			read(0, &c, 1);
			if	( c == 27 )	{
				unsigned char buf[8];
				int		b, i;

				b = read(0, buf, 8);
				c = 0;
				for ( i = 0; i < b; i ++ )	
					c = (c << 8) | buf[i];	// produce unique code of unix keys
				}
			break;
			}
		} while ( c == 0 && waitf );

	//
	tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
	return c;
}

/* input for a character (with check about valid chars) */
char	cons_cinp(const char *prompt, const char *valid)
{
	int		ch;

	cons_pratf(1, screen_height, "%s>>> %s (%s) ? %s", clr_hdrhi, prompt, valid, clr_reset);
	do	{
		ch = cons_getkey(1);
		if	( ch == '/' )					ch = '\1';
		if	( ch == '\n' )					ch = 'y';
		if	( ch == 27 )	{
			if	( strchr(valid, 'q') )
				ch = 'q';
			else
				ch = 'n';
			}
		} while ( strchr(valid, ch) == NULL );

	printf("\r\033[K");
	return ch;
}

/* clear screen */
void	cons_cls()
{	printf("%s\033[2J\033[1;1H", clr_reset);	}

/* move cursor at specified position */
void	cons_gotoxy(int x, int y)
{	printf("\033[%d;%dH", y+1, x+1);			}

/* enable cursor */
void	cons_enable_cursor()		
{	
	if	( strcmp(getenv("TERM"), "linux") == 0 )
		printf("\033[?25h\033[?8c");	
}

/* disable cursor */
void	cons_disable_cursor()
{
	if	( strcmp(getenv("TERM"), "linux") == 0 )
		printf("\033[?25l\033[?1c");	
}

/* gets a string from console */
char	*cons_input(const char *prompt, const char *source=NULL)
{
	static char buf[1024];
	int		ch;
	int		pos = 0;
	bool	replace_mode = false;
	bool	first_key = true;

	memset(buf, 0, 1024);
	if	( source )
		strcpy(buf, source);

	do	{
		cons_pratf(1, screen_height, "%s\033[K>>> %s ? %s%s", clr_hdrhi, prompt, clr_reset, buf);
		if	( pos < strlen(buf) )
			printf("\033[%dD", strlen(buf)-pos);
		fflush(stdout);
		ch = cons_getkey(1);
		if	( first_key )	{
			first_key = false;
			if	( (ch >= 32) && (ch <= 255) && (ch != '\b') && (ch != '\x1F') )	{
				// erase
				memset(buf, 0, 1024);
				pos = 0;
				}
			}

		switch ( ch )	{
		case	'\n': case	'\r':
			break;
		case	'\033':
			*buf = '\0';
			break;
		case 5976446:	// home
			pos = 0;
			break;
		case 5977214:	// end
			pos = strlen(buf);
			break;
		case 127: 		// backspace
		case   8:
			if	( pos > 0 )		{
				if	( buf[pos-1] )	{
					int remain = strlen((char *) (buf+pos));
					char *l = (char *) malloc(remain+1);
					strcpy(l, buf+pos);			 

					buf[pos-1] = '\0';
					strcat((char *) buf, (char *) l);
					free(l);

					pos --;
					}
				}
			else	{
				printf("\a");
				fflush(stdout);
				}
			break;

		case 5976958: 		// delete
			if	( pos < strlen(buf) )		{
				if	( buf[pos] )	{
					int remain = strlen((char *) (buf+pos+1));
					char *l = (char *) malloc(remain+1);
					strcpy(l, buf+pos+1);

					buf[pos] = '\0';
					strcat((char *) buf, (char *) l);
					free(l);
					}
				}
			else	{
				printf("\a");
				fflush(stdout);
				}
			break;

		case 5976702:	// insert
			replace_mode = !replace_mode;
			break;

		case	23364:	// right
			if	( pos > 0 )
				pos --;
			else	{
				printf("\a");
				fflush(stdout);
				}
			break;

		case	23363:	// right
			if	( pos < strlen(buf) )
				pos ++;
			else	{
				printf("\a");
				fflush(stdout);
				}
			break;

		default:
			if	( ch < 0xFF && ch > 31 )	{
				if	( replace_mode )	{
					// overwrite mode
					if	( pos >= strlen(buf) )	{
						buf[pos] = ch;
						buf[pos+1] = '\0';
						}
					else
						buf[pos] = ch;
					}
				else	{
					// insert mode
					int remain = strlen((char *) (buf+pos));
					char *l = (char *) malloc(remain+1);
					strcpy(l, buf+pos);
					buf[pos] = ch;
					buf[pos+1] = '\0';
					strcat((char *) buf, (char *) l);
					free(l);
					}

				pos ++;
				}
			else	{
				printf("\a");
				fflush(stdout);
				}
			};

		} while ( strchr("\n\r\033", ch) == NULL );

	printf("\r\033[K");
	return buf;
}

/* --- Menu Nodes ---------------------------------------------------------------------------------------- */

class InfoMenu	{
public:
	
	class InfoMenuItem	{
	public:
		char	*text;				// the text
		int		x, y, len;			// position and length on this page

		InfoMenuItem *next;			// next ptr on the list

	public:
		// default constructor
		InfoMenuItem()
					{ (text = NULL, x = y = len = 0, next = NULL); }

		// constructor: build a full item
		InfoMenuItem(int ix, int iy, const char *str)
					{
					x = ix; y = iy; 
					len = strlen(str);
					if	( len )	{
						text = new char[len+1];
						strcpy(text, str);
						}
					else
						text = NULL;
					next = NULL;
					}

		// destructor
		virtual ~InfoMenuItem() { if ( text ) delete[] text; }
		};

	// the menu-items list
	InfoMenu::InfoMenuItem	*head, *tail;

	// the number of items in the list
	int		count;

	// the selected item
	InfoMenu::InfoMenuItem	*selected;

public:
	InfoMenu()						{ head = tail = selected = NULL; count = 0; }
	virtual 	~InfoMenu()			{ clear(); }

	void		clear();								// reset
	void		add(int x, int y, const char *text);	// add a new item
	bool		select(int item);						// select an item (by position)
	bool		select(const char* item);				// select an item (by name)
	const char*	getSelectedKey() const					// returns the selected info-node key
					{ return ((selected) ? selected->text : NULL); }

	bool		selectFirst()
				{ return select(0); }
	bool		selectNext()
				{ return select(getNodeN(selected) + 1); }
	bool		selectPrev()		
				{ return select(getNodeN(selected) - 1); }

	InfoMenu::InfoMenuItem*
			getNode(int n) const;					// return's the nth node or NULL
	InfoMenu::InfoMenuItem*
			getNode(const char *name) const;		// return's the nth node or NULL

	int		getNodeN(InfoMenu::InfoMenuItem *node) const;	// return the number of node
	};

/* reset menu-table */
void	InfoMenu::clear()
{
	InfoMenu::InfoMenuItem	*cur, *pre;

	cur = head;
	while ( cur )	{
		pre = cur;
		cur = cur->next;
		delete pre;
		}
	head = tail = selected = NULL;	
	count = 0;
}

/* returns the nth node */
InfoMenu::InfoMenuItem*	InfoMenu::getNode(int n) const
{
	InfoMenu::InfoMenuItem	*cur;
	int		i;

	for ( i = 0, cur = head; i < n && cur; i ++, cur = cur->next );
	return cur;
}

// return the number of node
int		InfoMenu::getNodeN(InfoMenu::InfoMenuItem *node) const
{
	InfoMenu::InfoMenuItem	*cur = head;
	int		i = 0;

	if	( !node )
		return -1;

	while ( cur )	{
		if	( cur == node )
			return i;

		i ++;
		cur = cur->next;
		}

	return -1;
}

/* returns the node with the same name */
InfoMenu::InfoMenuItem*	InfoMenu::getNode(const char *name) const
{
	InfoMenu::InfoMenuItem	*cur = NULL;

	if	( name )	{
		cur = head;
		while ( cur )	{
			if	( strcmp(name, cur->text) == 0 )
				return cur;
			cur = cur->next;
			}
		}
	return cur;
}

/* add menu-item */
void	InfoMenu::add(int x, int y, const char *str)
{
	InfoMenu::InfoMenuItem	*newItem = new InfoMenu::InfoMenuItem(x, y, str);

	if	( !head )
		( head = tail = newItem );
	else
		( tail->next = newItem, tail = newItem );
	count ++;
}

/* select an item */
bool	InfoMenu::select(int item)
{
	if	( item >= 0 && item < count )	{
		// unselect previous
		if	( selected )	{
			cons_gotoxy(selected->x-1, selected->y-1);
			printf("%s%s%s", clr_menu, selected->text, clr_reset);
			}

		// select
		selected = getNode(item);
		if	( selected )	{
			cons_gotoxy(selected->x-1, selected->y-1);
			printf("%s%s%s\033[%dD", clr_mnusel, selected->text, clr_reset, strlen(selected->text)+1);
			}
		}
	else
		return false;
	return true;
}

/* select an item */
bool	InfoMenu::select(const char *name)
{
	InfoMenu::InfoMenuItem	*node;

	if	( (name != NULL) && (node = getNode(name)) )	{
		// unselect previous
		if	( selected )	{
			cons_gotoxy(selected->x-1, selected->y-1);
			printf("%s%s%s", clr_menu, selected->text, clr_reset);
			}

		// select
		selected = node;
		if	( selected )	{
			cons_gotoxy(selected->x-1, selected->y-1);
			printf("%s%s%s\033[%dD", clr_mnusel, selected->text, clr_reset, strlen(selected->text)+1);
			}
		}
	else
		return false;
	return true;
}

// GLOBAL Menu
static InfoMenu	menu;

/* --- Navigation stack (previous nodes, left-key) ------------------------------------------------------- */

static char	fix_menu_pos[1024];		// bug fix: menu position after nav.pop

class InfoNavStack	{
public:

	class InfoPos {
	public:
		char	*nodeKey;

		// current view
		int		pageTop, pageSize;
		char	*menuSelectedKey;

		InfoPos	*next;

	public:
		InfoPos()
			{
				pageTop = pageSize = 0; 
				nodeKey = menuSelectedKey = NULL; 
				next = NULL; 
			}

		virtual ~InfoPos()
			{
				if ( nodeKey ) delete[] nodeKey; 
				if ( menuSelectedKey ) delete[] menuSelectedKey;
			}
		};

	//
	InfoNavStack::InfoPos	*head;
	
public:
	InfoNavStack()
		{	head = NULL;	}

	virtual ~InfoNavStack()
		{
			clear();
		}

	void	clear();

	void	push();
	void	pop();
	};

/* reset stack */
void	InfoNavStack::clear()
{
	InfoNavStack::InfoPos	*cur, *pre;

	cur = head;
	while ( cur )	{
		pre = cur;
		cur = cur->next;
		delete pre;
		}
	head = NULL;	
}

// store inf-view position
void	InfoNavStack::push()
{
	InfoNavStack::InfoPos	*npos = new InfoNavStack::InfoPos;

	npos->nodeKey = new char[strlen(cur_node_name)+1];
	strcpy(npos->nodeKey, cur_node_name);
	npos->pageTop = cur_page_top;
	npos->pageSize = max_page_size;
	if	( menu.getSelectedKey() )	{
		npos->menuSelectedKey = new char[strlen(menu.getSelectedKey())+1];
		strcpy(npos->menuSelectedKey, menu.getSelectedKey());
		}

	// connect
	if	( !head )
		head = npos;
	else
		( npos->next = head, head = npos );
}

// restore inf-view position
void	InfoNavStack::pop()
{
	if	( head )	{
		InfoNavStack::InfoPos	*cur;

		// pop node		
		cur  = head;
		head = head->next;

		// set back info-viewer
		strcpy(cur_node_name, cur->nodeKey);
		cur_page_top  = cur->pageTop;
		max_page_size = cur->pageSize;
		if	( cur->menuSelectedKey )	{
//			menu.select(cur->menuSelectedKey);
			strcpy(fix_menu_pos, cur->menuSelectedKey);
			}
		else
			strcpy(fix_menu_pos, "");

		// delete node
		delete cur;
		}
}

// GLOBAL NavStack
static InfoNavStack		navStack;

/* --- Application --------------------------------------------------------------------------------------- */

// page header
void	header(const char *file, const char *fmt, ...)
{
	va_list	ap;
	char	msg[2048];
	int		i;

	va_start(ap, fmt);
	vsnprintf(msg, 2048, fmt, ap);
	va_end(ap);

	printf("%s", clr_header);
	for ( i = 0; i < screen_width; i ++ )		printf("_");
//	printf("\r`%s'", cur_node_name);
	printf("\r_______%s * %s * %s", clr_hdrhi, cur_node_name, clr_reset);
	printf("\033[%dG %s%s", (screen_width - strlen(msg)), clr_hdrhi, msg);
	printf("%s", clr_reset);
	printf("\n");
}

// page footer
void	footer(const char *fmt, ...)
{
	va_list	ap;
	char	msg[2048];
	int		i;

	va_start(ap, fmt);
	vsnprintf(msg, 2048, fmt, ap);
	va_end(ap);

	cons_gotoxy(0, screen_height-2);
	printf("%s", clr_footer);
	for ( i = 0; i < screen_width; i ++ )		printf("_");
	printf("\n\033[K%s", clr_hdrhi);
	cons_rprintf(msg);
	printf("%s", clr_reset);
}

// print nav-points
void	print_info_header(Info::InfoNode *node)
{
 	if	( !node->strPrev && !node->strNext && node->strUp )
		header(node->infoFile, "(U)p: %s ", node->strUp);
	else if	( node->strPrev && node->strNext && !node->strUp )
		header(node->infoFile, "(P)rev: %s, (N)ext: %s ", node->strPrev, node->strNext);
	else if	( !node->strPrev && node->strNext && node->strUp )
		header(node->infoFile, "(N)ext: %s, (U)p: %s ", node->strNext, node->strUp);
	else if	( node->strPrev && !node->strNext && node->strUp )
		header(node->infoFile, "(P)rev: %s, (U)p: %s ", node->strPrev, node->strUp);
	else if	( !node->strPrev && !node->strNext && !node->strUp )
		header(node->infoFile, "Linux Help Center ");
	else
		header(node->infoFile, "(P)rev: %s, (N)ext: %s, (U)p: %s ", node->strPrev, node->strNext, node->strUp);
}

//  clear page's area
void	clear_page()
{
	int		y = 0;
	int		maxlines = screen_height - 2;

	for ( y = 0; y < maxlines; y ++ )	{
		cons_gotoxy(0, y+1);
		printf("%s\033[K", clr_reset);
		}
}

// display an info-line
void	print_info_line(Info::InfoLine *line, int y)
{
	char	buf[1024];
	Info::InfoElem	*cur = line->head;

	printf("%s%04d:%s", clr_lines, y+cur_page_top, clr_reset);

	if	( page_margin )
		printf("\033[%dG", page_margin+1);

	printf("%s", line->text);

	while ( cur )	{
		int		dif;

		dif = (cur->pos - line->text);
		strcpy(buf, cur->pos);
		buf[cur->len] = '\0';

		if	( cur->type == Info::ie_menu || cur->type == Info::ie_note ) {
			printf("\033[%dG%s%s%s", dif+2+page_margin, clr_menu, buf+1, clr_reset);
			menu.add(dif+2+page_margin, y+1+top_margin, buf+1);
			}
		else if ( cur->type == Info::ie_bold )
			printf("\033[%dG%s%s%s", dif+1+page_margin, clr_bold, buf, clr_reset);
		else if ( cur->type == Info::ie_italics )
			printf("\033[%dG%s%s%s", dif+1+page_margin, clr_italics, buf, clr_reset);

		cur = cur->next;
		}

	printf("\n");
}

// draw an info page
void	draw_info()
{
	Info::InfoNode	*node;
	int		y = 0, ign = cur_page_top, i;
	int		maxlines = screen_height - (4+top_margin);

	clear_page();
	menu.clear();

	node = inf.findNode(cur_node_name);
	max_page_size = 1;
	if	( node )	{
		Info::InfoLine	*line;

		cons_gotoxy(0, 0);
		print_info_header(node);

		for ( i = 0; i < top_margin; i ++ )
			printf("\n");

		line = node->head;
		while ( line )	{
			max_page_size ++;

			if	( ign )
				ign --;
			else if	( y < maxlines )	
				print_info_line(line, ++ y);

			line = line->next;
			}

		if	( max_page_size-1 > (cur_page_top + maxlines) )
			printf("%s...continued...%s", clr_lines, clr_reset);
		else
			printf("%s...eof...%s", clr_lines, clr_reset);

		footer("menu: cursor-keys,h,o,s,/,n,p,u,t,q - %s", node->infoFile);
		}
	else	{
		clear_page();
		cons_pratf(4, 3, "`%s'-node not found", cur_node_name);
		}

	//
	if	( strlen(fix_menu_pos) )	{
		if	( !menu.select(fix_menu_pos) )
			menu.selectFirst();
		}
	else
		menu.selectFirst();
}

//
void	help()
{
	char	*help_str =
	"ninf... info/man page viewer\n"
	"Nicholas Christopoulos\n"
	"\n"
	"q\t\tquit\n"
	"o\t\topen another file\n"
	"h\t\tthis screen\n"
	"t,u,p,n\t\tjump to top, up, previous, next node\n"
	"s or /\t\tsearch\n"
	"pgup/pgdn\tpage up/down\n"
	"left,backspace\tback\n"
	"up,down,enter\tmenu prev/next, select\n"
	;

	clear_page();
	cons_gotoxy(0, 2);
	printf("%s%s", clr_reset, help_str);
	footer("press any key - ninf");
	cons_getkey(1);
	draw_info();
}

//
static char	env_manwidth[64];

//
int	main(int argc, char *argv[])
{
	int		c;
	Info::InfoNode	*node;
	Info::InfoLine	*start_search = NULL, *infline_found;

	cons_init();

	// set width for man-pages
	sprintf(env_manwidth, "MANWIDTH=%d", screen_width - 6);
	putenv(env_manwidth);

	// default node 'Top'
	strcpy(cur_node_name, "Top");
	cur_page_top  = 0;
	max_page_size = 1;

	//
	if	( argc > 1 )	{
		// load file
		if	( !inf.loadInfo(argv[1]) )
			if	( !inf.loadMan(argv[1]) )
				inf.loadText(argv[1]);

		cons_disable_cursor();

		// select page
		draw_info();

		do	{
			c = cons_getkey(1);
			switch ( c )	{
			case	'q': case 'Q':
				c = 'q';
				break;
			case	'h': case 'H':
				help();
				break;
			case	'o': case 'O':
				{
					char	newfile[256];

					cons_enable_cursor();
					strcpy(newfile, cons_input("Open file"));
					cons_disable_cursor();
					if	( strlen(newfile) )	{
						inf.clear();
						if	( !inf.loadInfo(newfile) )
							if	( !inf.loadMan(newfile) )
								inf.loadText(newfile);

						// default node 'Top'
						strcpy(cur_node_name, "Top");
						cur_page_top  = 0;
						max_page_size = 1;

						draw_info();
						}
				}
				break;
			case	's': case 'S':	// search
			case	'/':
				{
					char	old_search_string[1024];

					strcpy(old_search_string, last_search_string);

					cons_enable_cursor();
					strcpy(last_search_string, cons_input("Search for", last_search_string));
					cons_disable_cursor();

					if	( strlen(last_search_string) )	{
						if	( strcmp(last_search_string, old_search_string) != 0 ) // new search string
							start_search = NULL;		// search from the begin

						infline_found = inf.findString(last_search_string, start_search);
						if	( infline_found )	{
							node = (Info::InfoNode*) infline_found->parent;		// found in this node
							if	( node )	{
								navStack.push();

//								if	( node->next )	
//									start_search = node->next->head;			// next search to next node

								if	( node->name )	{							// update view
									strcpy(cur_node_name, node->name);
									cur_page_top = 0;
									draw_info();
									cons_pratf(0, screen_height, "%s\033[K* found at `%s'", clr_reset, node->name);
									}
								}
							}
						else
							cons_pratf(0, screen_height, "%s\033[K* not found *", clr_reset);
						}
					else	// cancel
						strcpy(last_search_string, old_search_string);
				}
				break;

			case	'u': case 'U':	// up node
			case	'^':
				if	( (node = inf.findNode(cur_node_name)) )	{
					if	( node->strUp )	{
						navStack.push();
						strcpy(cur_node_name, node->strUp);

						cur_page_top = 0;
						draw_info();
						}
					}
				break;
			case	'n': case 'N':	// next node
			case	'>':
				if	( (node = inf.findNode(cur_node_name)) )	{
					if	( node->strNext )	{
						navStack.push();
						strcpy(cur_node_name, node->strNext);

						cur_page_top = 0;
						draw_info();
						}
					}
				break;
			case	'p': case 'P':	// previous node
			case	'<':
				if	( (node = inf.findNode(cur_node_name)) )	{
					if	( node->strPrev )	{
						navStack.push();
						strcpy(cur_node_name, node->strPrev);

						cur_page_top = 0;
						draw_info();
						}
					}
				break;
			case	't': case 'T':	// top node
				navStack.push();
				strcpy(cur_node_name, "Top");

				cur_page_top = 0;
				draw_info();
				break;
			case	'\b':	case	'\x1F':
			case	23364:	// left
				navStack.pop();
				draw_info();
				break;
			case	23361:	// up
				menu.selectPrev();
				break;
			case	23362:	// down
			case	23363:	// right
				menu.selectNext();
				break;
			case	5977470:	// page up
				cur_page_top -= (screen_height - (4+top_margin));
				if	( cur_page_top < 0 )
					cur_page_top = 0;

				draw_info();
				break;
			case	5977726:	// page down
				cur_page_top += (screen_height - (4+top_margin));
				if	( cur_page_top > max_page_size )
					cur_page_top = max_page_size - 1;

				draw_info();
				break;
			case	10:	case	13:	// enter
				if	( menu.getSelectedKey() )	{
					navStack.push();
					strcpy(cur_node_name, menu.getSelectedKey());

					cur_page_top = 0;
					draw_info();
					}
				break;
			default:
				cons_pratf(0, screen_height, "unknown key: %d    ", c);
				}
			} while ( c != 'q' );

		// return
		cons_enable_cursor();
		cons_gotoxy(0, screen_height-1);
		printf("\033[KThank you :)\n");
		}
	else
		printf("usage: ninf [info|man|text-file]\n");

	return 1;
}

