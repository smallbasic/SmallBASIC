/**
*/

#if !defined(_tedbcb_h)
#define _tedbcb_h

#include <vcl.h>
#include "../editor/ted.h"

class TedBCB : public TED	{
private:
	int		cx, cy;		// cursor pos
	int		lcx, lcy;	// cursor pos (last draw)
    int		fx, fy;		// font size
    int		last_tc;

    int		k_buf[256];
    int		k_head, k_tail;
    bool	store_keys;

	char	font_name[256];
    int		font_size;
    
	int		dev_input_remove_char(byte *dest, int pos);

    bool	lock_refresh;

	Graphics::TBitmap	*bmp;
    TCanvas		*canvas;
    TWinControl	*win;
    TPaintBox	*img;

public:
	TedBCB(TPaintBox *img);
    virtual ~TedBCB();

    bool	Enabled;

	// pseudo-terminal
	void	clrscr();							// clear screen
	void	clreol();							// clear to EOL
	void	gotoxy(int x, int y);				// set cursor position
    void	getxy(int *x, int *y)	{ *x = cx; *y = cy; }	// get cursor position
    void	deleteln();							// delete the line at cursor and scroll up the rests
    void	insertln();							// insert an empty line at cursor and scroll down the rests
	void	printw(const char *fmt, ...);		// printf    
	int		get_pref_ch();						// gets a key (similar to getch() but for extended keys uses > 0x100)
	char	*get_string(const char *defval=NULL);	// get a string from console, returns a static string, str[0] = 0 for escape
	int		scr_rows();							// returns the number of the rows of the window
	int		scr_cols();							// returns the number of the columns of the window
	void	beep();								// just produce a beep
	void	addch(int ch);						// print a char at current position
	void	refresh();							// flush screen
	void	sys_exit();							// Alt+X
    void	set_color(int color);

    // notify
	void	clip_copy();
    void	clip_paste();
    
    // 
	void	status_line(const char *say);
	char*	ask(const char *prompt, char *buf, int size = 256, const char *defval = NULL);
    
    // keyboard buffer
    void	push_key(int c, int mcas = 0);
    int		inkey();

	void	set_font(const char *font, int size);
    void	update_cursor();

    // paint
    void	paint();

    // 
    int		font_cx() const		{ return fx; }
    int		font_cy() const		{ return fy; }
	};

#endif
