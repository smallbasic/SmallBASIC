/**
*/

#if !defined(_tedbcb_h)
#define _tedbcb_h

//typedef unsigned char byte;
#define byte unsigned char
//typedef unsigned int int32;

#include "ted.h"
#include "dev_term.h"

class TedDT : public TED	{
private:
    int		last_tc;				// last key-code

	// fonts
	char	font_name[256];
    int		font_size;

    bool	lock_refresh;			// lock refresh
	bool	repaint;				// redraw screen in the next refresh

public:
	bool	finished;				// true, editor exits

public:
	TedDT();
    virtual ~TedDT();

	// pseudo-terminal
	void	clrscr();							// clear screen
	void	clreol();							// clear to EOL
	void	gotoxy(int x, int y);				// set cursor position
	void	getxy(int *x, int *y);				// get cursor position
    void	deleteln();							// delete the line at cursor and scroll up the rests
    void	insertln();							// insert an empty line at cursor and scroll down the rests
	void	printw(const char *fmt, ...);		// printf    
	int		get_pref_ch();						// gets a key (similar to getch() but for extended keys uses > 0x100), translated to TED's keys
	int		get_row_ch();						// gets a key (similar to getch() but for extended keys uses > 0x100)
	int		scr_rows();							// returns the number of the rows of the window
	int		scr_cols();							// returns the number of the columns of the window
	void	beep();								// just produce a beep
	void	addch(int ch);						// print a char at current position
	void	refresh();							// flush screen
    void	set_color(int color);				// set color (VGA16)

    // clipboard notify
	void	clip_copy();
    void	clip_paste();

	// keyboard
	int		translate(int ch);
	int		key(int ch);

	//
	void	set_font(const char *font, int size);
    void	update_cursor();

	// user commands
	void	help_on_word(const char *word);
	void	sys_exit();							// Alt+X

    // paint
    void	paint();
    void	realrefresh();
	};

#endif
