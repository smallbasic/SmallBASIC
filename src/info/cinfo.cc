/**
*/

#include "info.h"
#include <ncurses.h>

// --- Colors ---------------------------------------------------------

#define	CPR_DESKTOP		1
#define	CPR_WINDOW		1
#define	CPR_HELPWIN		2
#define	CPR_MSGWIN		3
#define	CPR_MENU		4
#define	CPR_STATUS		4

#define	CPR_HELPLINK	5

// --- Event ----------------------------------------------------------

// event type
enum event_t {
	evNil,
	evMouseClick,
	evMouseDoubleClick,
	evKey,
	evEOL
	};

// event
class Event	{
public:
	enum event_t type;
	
	int		x, y;

	int		keyCode;
	int		mouseButtons;
	};

// --- Window ---------------------------------------------------------

// window
class Window	{
protected:
	WINDOW	*win;
	char	*title;

public:
	Window	*next;		// next window ptr

public:
	Window(int color=CPR_WINDOW);
	virtual	~Window();

	void	printAt(int x, int y, const char *text);
	void	clear()
				{ wclear(win); }
	void	flush()
				{ wrefresh(win); }
	virtual void handleEvent(const Event &ev)
				{ }

	void	setTitle(const char *newTitle);

	virtual	void draw();
	};

// constructor
Window::Window(int color)
{
	next = NULL;
	title = NULL;

	win = newwin(LINES-6, COLS-6, 2, 2);
    wbkgd(win, COLOR_PAIR(color));
}

// destructor
Window::~Window()
{
	if	( title )
		delete[] title;
	delwin(win);
}

//
void	Window::draw()
{
	werase(win);
	box(win, ACS_VLINE, ACS_HLINE);
	if	( title )
		printAt(2, 0, title);
	wrefresh(win);
}

//
void	Window::setTitle(const char *newTitle)
{
	if	( title )
		delete[] title;

	title = new char[strlen(newTitle)+1];
	strcpy(title, newTitle);
}

// print
void	Window::printAt(int x, int y, const char *text)
{
	mvwaddstr(win, y, x, text);
}

// --- InfoWindow -----------------------------------------------------

class InfoWin : public Window {
private:
	Info	inf;

	void	printInfoLine(int x, int y, Info::InfoLine *line);
			 
public:
	InfoWin(const char *file);
	virtual ~InfoWin()	{ }

	void	draw();
	};

//
InfoWin::InfoWin(const char *file) : Window(CPR_HELPWIN)
{
	inf.loadInfo(file);
	setTitle(file);
}

//
void	InfoWin::printInfoLine(int x, int y, Info::InfoLine *line)
{
	Info::InfoElem	*cur = line->head;

	printAt(x, y, line->text);

	while ( cur )	{
		int		dif;

		dif = (cur->pos - line->text);
		if	( cur->type == Info::ie_menu || cur->type == Info::ie_note ) 
			mvwchgat(win, y, x+dif, cur->len, A_BOLD, COLOR_PAIR(CPR_HELPLINK), NULL);
		else	
			mvwchgat(win, y, x+dif, cur->len, A_BOLD, COLOR_PAIR(CPR_HELPWIN), NULL);

		cur = cur->next;
		}
}

//
void	InfoWin::draw()
{
	Info::InfoNode	*node;
	int		y = 1;

	Window::draw();
	node = inf.findNode("Top");
	if	( node )	{
		Info::InfoLine	*line;

		line = node->head;
		while ( line )	{
			printInfoLine(2, y ++, line);
			line = line->next;
			}
		}
	else
		printAt(2, 2, "Top-node not found");

	flush();
}

// --- WinManager -----------------------------------------------------

class WinManager {
private:
	Window	*winHead, *winTail;

public:
	WinManager();
	virtual	~WinManager();

	void	addWindow(Window *win);
	int		count();
	void	handleEvent(const Event &ev);
	void	draw();
	};

// constructor
WinManager::WinManager()
{
	winHead = winTail = NULL;
}

// deconstructor
WinManager::~WinManager()
{
	Window	*pre, *cur = winHead;

	while ( cur )	{
		pre = cur;
		cur = cur->next;
		delete pre;
		}
}

// return the number of windows
int		WinManager::count()
{
	Window	*cur = winHead;
	int		cnt = 0;

	while ( cur )	{
		cur = cur->next;
		cnt ++;
		}
	return cnt;
}

// redraw all
void	WinManager::draw()
{
	Window	*cur = winHead;

	while ( cur )	{
		cur->draw();
		cur = cur->next;
		}
}

// add a new window into the list
void	WinManager::addWindow(Window *win)
{
	if ( !winHead )
		( winHead = winTail = win );
	else
		( winTail->next = win, winTail = win );
	win->draw();
}

//
void	WinManager::handleEvent(const Event &ev)
{
	Window	*cur = winHead;

	while ( cur )	{
		cur->handleEvent(ev);
		cur = cur->next;
		}
}

// --- Desktop --------------------------------------------------------

class Desktop {
private:
	WinManager	*winMgr;
	WINDOW		*win;
	WINDOW		*winMenu;
	WINDOW		*winStatus;

public:
	Desktop();
	virtual ~Desktop();

	void	addWindow(Window *win)		{ winMgr->addWindow(win); }
	void	eventLoop();
	void	draw();
	};

// constructor
Desktop::Desktop()
{
	initscr();
	cbreak();
	noecho();
	start_color();

    init_pair(CPR_DESKTOP,  COLOR_WHITE,  COLOR_BLUE);
    init_pair(CPR_HELPWIN,  COLOR_BLACK,  COLOR_CYAN);
    init_pair(CPR_MSGWIN,   COLOR_BLACK,  COLOR_WHITE);
    init_pair(CPR_STATUS,   COLOR_BLACK,  COLOR_WHITE);
    init_pair(CPR_HELPLINK, COLOR_YELLOW, COLOR_CYAN);

	win = newwin(LINES, COLS, 0, 0);
	winMenu = newwin(1, COLS, 0, 0);
	winStatus = newwin(1, COLS, LINES-1, 0);

	wbkgd(win, COLOR_PAIR(CPR_DESKTOP));
    wbkgd(winMenu, COLOR_PAIR(CPR_STATUS));
    wbkgd(winStatus, COLOR_PAIR(CPR_STATUS));

	werase(win);
	werase(winMenu);
	werase(winStatus);

	winMgr = new WinManager;

	draw();
}

// destructor
Desktop::~Desktop()
{
	delete winMgr;

	delwin(winStatus);
	delwin(winMenu);
	delwin(win);
	endwin();
}

// draw desktop
void	Desktop::draw()
{
	wclear(win);
	wrefresh(win);
    mvwaddstr(win, 6, 2, "This line shouldn't appear");
    wrefresh(win);

    wrefresh(winMenu);
    wrefresh(winStatus);
}

// main event-loop
void	Desktop::eventLoop()
{
	int		c;
	Event	ev;

    mousemask(ALL_MOUSE_EVENTS, NULL);
	do	{
//		wrefresh(win);
		c = wgetch(win);

		if	( c == KEY_MOUSE )	{
			MEVENT	mcev;

			getmouse(&mcev);

			if	( mcev.id == BUTTON1_CLICKED )
				ev.type = evMouseClick;
			else if	( mcev.id == BUTTON1_DOUBLE_CLICKED )
				ev.type = evMouseDoubleClick;
			else	
				ev.type = evNil;

			ev.x = mcev.x;
			ev.y = mcev.y;
			}
		else	{
			ev.type = evKey;
			ev.keyCode = c;
			getyx(stdscr, ev.y, ev.x);

			if	( c == 'q' )
				break;
			if	( c == 'r' )	
				winMgr->draw();
			}

		if	( ev.type != evNil )	
			winMgr->handleEvent(ev);

		} while ( true );
}

// --- main() ---------------------------------------------------------

Desktop	*desk;

int	main(int argc, char *argv[])
{
	int		i;

	desk = new Desktop();
	for ( i = 1; i < argc; i ++ )
		desk->addWindow(new InfoWin(argv[i]));
	desk->eventLoop();
	delete desk;

	return 1;
}

