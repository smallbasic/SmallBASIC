/**
*	Encapsulation class of TED
*
*	The 'TED' is an text editor for Unices written by Fernando Joaquim Ganhao Pereira . 
*
*	The system drivers and the class written by	Nicholas Christopoulos.
*	Fernado had done a good job, but there are no comments...
*	I am still looking how it is works.
*
*	both are under GPL license.
*/

#if !defined(_ted_sb_h)
#define _ted_sb_h

#include <string.h>

#define	TED_LSZ			4096	// 256
#define	TED_MKEYS		128		// 12
#define	TED_MAX_WAREAS	64		// 21
#define	TED_UNDO_NODES	512		// ndc: undo nodes per buffer, this can eat a lot of memory but I don't care

#define TEDK_RETURN		TEDK_ENTER	
enum ted_key {
TEDK_TAB    = 9,
TEDK_ESCAPE = 27,
TEDK_ENTER  = 13,
TEDK_BSPACE = 8,	// delete one char back
TEDK_TEDK = 0x100,
TEDK_DEFKEY,	// define key
TEDK_INSOVR,	// insert/overwrite switch
TEDK_INDENT,	// auto-ident switch
TEDK_SENSIT,	// case sensitivity switch
TEDK_CHCASE,	// change case
TEDK_MATCH,		// match
TEDK_DELCHAR,	// delete front-char
TEDK_DELSEL,	// delete selected
TEDK_COPYSEL,	// copy and select
TEDK_PASTE,		// paste
TEDK_MARK0,		// bookmark 
TEDK_MARK1,		// bookmark 
TEDK_MARK2,		// bookmark 
TEDK_MARK3,		// bookmark 
TEDK_MARK4,		// bookmark 
TEDK_MARK5,		// bookmark 
TEDK_MARK6,		// bookmark 
TEDK_MARK7,		// bookmark 
TEDK_MARK8,		// bookmark 
TEDK_MARK9,		// bookmark 
TEDK_GMARK,		// goto bookmark
TEDK_GOTO,		// go to line
TEDK_SEARCH,
TEDK_FPREV,		// direction = !direction; find next
TEDK_FNEXT,		// find next
TEDK_REPEAT,	// repeat ???
TEDK_SUBST,		// substitute();
TEDK_REFRESH,	// refresh screen
TEDK_PREVSCR,	// page up?
TEDK_BOF,		// go to line 0
TEDK_NEXTSCR,	// page down?
TEDK_EOF,		// go to end
TEDK_KILL,		// kill line
TEDK_WORD,		// one word forward
TEDK_DELWORD,	// delete next word
TEDK_BWORD,		// one word backward
TEDK_DELBWORD,	// delete back word
TEDK_UNDO,		// undo
TEDK_HOME,		// HOME
TEDK_DELBOL,	// delete to start of line
TEDK_EOL,		// END
TEDK_DELEOL,	// delete to end-of-line
TEDK_AGAIN,		// repeat_again();
TEDK_DOWN,		// DOWN
TEDK_LEFT,		// LEFT
TEDK_RIGHT,		// RIGHT
TEDK_UP,		// UP
TEDK_MENU,		// menu();
TEDK_COMMAND,	// parse_command();
TEDK_HELP,		// help(); 
TEDK_CENTER,	// ???
TEDK_ALIAS,		// ???
TEDK_DELETE,
TEDK_HELPWORD,
TEDK_NDC,		// my debug-key
TEDK_SELMARK,	// selection start
TEDK_LINESELMARK,	// selection start (line selection)
TEDK_SELCANCEL,	// cancel selection
TEDK_CUTSEL,	// Clipboard 'CUT'
TEDK_LINECUT,	// Brief's line-cut
TEDK_LINECOPY,	// Brief's line-copy
TEDK_BUFLIST,	// List of buffers (areas)
TEDK_SYSEXIT,	// normal exit
TEDK_SAVE,		// saves the file 
TEDK_LOAD,		// loads the file 
TEDK_CHANGEFN,	// change file-name
TEDK_NEXTBUF,	// activate the next buffer
TEDK_PREVBUF,	// activate the previous buffer
TEDK_NULL
};

// color codes
#if defined(_WinBCB)	// white background
#define	TED_BOOKMARK_COLOR		0xF9
#define	TED_XSCROLL_COLOR		0xFA
#define	TED_SELECTION_COLOR		0x07
#define	TED_TEXT_COLOR			0xF0
#define	TED_EMPTY_LINE_COLOR	0xF8
#define TED_FOOTER_COLOR		0xF8
#define TED_HEADER_COLOR		FOOTER_COLOR
#define TED_COLOR				0xF8
#define TED_SELCOLOR			0x8F
#define TED_STATUS_COLOR		0x8F

// syntax highlight
#define	TED_REMARK_COLOR		0xF2
#define	TED_COMMAND_COLOR		0xF1
#define	TED_STRING_COLOR		0xF3

#else	// black background

#define	TED_BOOKMARK_COLOR		0x19
#define	TED_XSCROLL_COLOR		0x1A
#define	TED_SELECTION_COLOR		0x31
#define	TED_TEXT_COLOR			0x1F
#define	TED_EMPTY_LINE_COLOR	0x1B
#define TED_FOOTER_COLOR		0x1F
#define TED_HEADER_COLOR		FOOTER_COLOR
#define TED_COLOR			0x1B
#define TED_SELCOLOR		0x31
#define TED_STATUS_COLOR		0x70

// syntax highlight
#define	TED_REMARK_COLOR		0x1A
#define	TED_COMMAND_COLOR		0x1E
#define	TED_STRING_COLOR		0x1B
#endif

class TED {
/***********************************************************************
*	TYPES
*/
private:

/**
*	text-line
*/
typedef struct line {
    char *l;
    struct line *prev, *next;
	} line;

/**
*	ndc: undo-node codes
*/
    enum undo_code_t {
	undo_null,		// nothing
	undo_cursor,	// cursor movement
	undo_line,		// line was changed; the ptr keeps the previous text
	undo_block,		// whole block of lines was changed; the ptr keeps the previous block
	undo_big_block,	// whole block of lines was changed; the ptr keeps the name of a temporary file
	undo_rmblock,	// block of lines was inserted, remove them
	undo_clipboard,	// restore clipboard's data
	undo_rmline		// remove this line
	};

/**
*	ndc: undo-node
*/
	typedef struct {
		undo_code_t	code;				
		int		col, row;		// column and line
	int		len;
	    char	*ptr;			// copy of text or NULL
	int		count;			// exec 'count' more elements
		} undo_node_t;

/**
*	text-file (buffer)
*/
typedef struct w_area {
    line _head;				// head-node of text
    line *_curr;			// current line ptr
    line *_screen_top;		// pointer to first line on view
    int _line_num;			// current line number ?
    int _buffer_size;		// number of nodes ?
    int _changed;			// modified flag ?
    int _x;					// cursor pos ?
    int _y;					//
    int _act_x;				// ???
    char _fname[1024];		// the filename
    int _sl_mark;			// selection line mark
    int _sx_mark;			// selection column mark

    // ndc: new undo system, cycling static list of undo nodes
    int			undo_head, undo_tail;
	undo_node_t	*undo_table;
    int _l_mark[10];		// ndc: bookmarks (lines)
    int _x_mark[10];		// ndc: bookmarks (columns)
	int	_hsyntax;			// ndc: syntax-table (0=none)
	} w_area;

/**
*	still looking for it
*/
typedef struct alias {
    char key[TED_MKEYS+1];
    char subs[4*TED_MKEYS+1];
	} alias;

/**
*	highlight node
*/
typedef struct {
	char	*sign;		// filename sign
	char	*ext;		// filename extention
	char	*csv;		// keywords, separated by '\n'

	char	desc[256];

	char	rem_block_b[8];	//	block remarks begin --- not working yet
	char	rem_block_e[8];	//	block remarks end --- not working yet
	char	rem_sym1[8];	//	line remarks (one char for now)
	char	rem_sym2[8];	//	line remarks
	char	rem_sym3[8];	//	line remarks
	char	strq_ch;		//	string character (\" or \')

	int		*index;			// index of ptrs
	int		count;			// number of words in csv
	} stx_t;

stx_t	*stx_table;

/**
*	...
*/
typedef struct kdef {
    int key;
    int multi;
    int meaning[TED_LSZ];
	} kdef;

/***********************************************************************
*	GLOBALS
*/
private:
	int		screen_lines;
	int		screen_cols;
	int		lat_shift;
	int		first_col;
	bool	indent;
	bool 	insert;
	bool	sensit;
	bool	direction;

    int		tab_size;
    int		jump_scroll;

    int		last_key;
    int		repeating_key;
    int		repeating_key_2;

	w_area 	wareas[TED_MAX_WAREAS];
	int 	current_warea;

	char 	undo_buffer[TED_LSZ];

	char 	def_macro[TED_LSZ];
	char 	clip_line[TED_LSZ];
	bool 	single_sel;
	line* 	last;
	line* 	last_page;
	int		n_alias;
	int		n_kdefs;
	bool	macro_changed;
	int 	def_macro_ndx;
	alias	*alias_list;
	kdef	*key_defs;

	int		pref_key;	// ???

	//////
	char	char_xscroll;
	char	char_empty;

	int		stx_count;

	int		sel_style;		// selection style, 0 = normal, 1 = line, later: 2 = column

	bool	in_editor_scr;	// true if editor is active screen

protected:
	char 	last_message[TED_LSZ];
	char 	last_usermsg[TED_LSZ];
	char	static_getstr[TED_LSZ];
	char    pattern[TED_LSZ];

/***********************************************************************
*	LOCAL ROUTINES
*/
private:
	const char	*basename(const char *file);
	void	delete_line(line *pos);
	line*	insert_newline(line *pos, char *str);
	void	redraw_line(int yl, char *ptr, int ln);
	int		real_col(int col, const char *str) const;
	bool	lateral_adjust(int col);
	void	find_next();
	void	search();
	int		find(const char *str, bool dir);
	void	substitute();
	void	subst_string(char *old, char *news);
	void	prev_screen();
	void	next_screen();
	void	home_col();
	void	delEOL();
	void	delBOL();
	void	advance_word();
	void	back_word();
	void	del_word();
	void	del_next_word();
	void	undo();
	int		move_down();
	int		move_left();
	int		move_right();
	int		move_up();
	void	delete_char();
	undo_node_t	*delete_backchar();
	void	overwrite(char c);
	void	insert_char(char c);
	void	new_line();
	void	scroll_up(char *l);
	void	scroll_down(char *l);
	int		get_val(char *str);
	void	repeat();
	void	rep_command(int n, int *cmd);
	int		f_instr(char *st1, char *source);
	int		r_instr(char *st1, char *source, int start);
	int		case_ins_strncmp(char *s1, char *s2, int n);
	void	parse_command();
	void	exit_and_save(bool realy_save);
	void	finish();
	void	create_alias(char *k, char *s);
	char 	*find_alias(char *key);
	int		subst_alias();
	int		define_key();
	void	key_definition(int key, int multi, int *def);
	void	save_defs(char *f_name);
	void	read_defs(char *f_name);
	void	print_defs();
	void	select_warea(int n);
	void	read_to_warea(char *string);
	void	read_buffer(int n, int li, int lf);
	void	write_warea(char *string);
	void	write_to_buffer(int n, int  li, int lf);
	void	delete_range(int li, int lf);
	void	erase_working_area(int n);
	void	mark(int);
	void	recall_mark();
//	void	delete_selected();
//	void	copy_and_select();
	line	*get_line_node(int ln);
	void	paste_buffer();
	void	match();
	void	user_buflist();
	int		wait_any_key();
	void	change_case();
	void	repeat_again();
	void	disp_status(const char *str);
	void	selmark();
	void	move(int y, int x)		{ gotoxy(x, y); }
	int		*get_cmd_str();
	char	*colfiles(char *dir, char *wc);
	void	filelist(const char *prefix);

    //
	int		input_remove_char(char *dest, int pos);
	void	ndc_console();
	char	*text_ln(int n);
	int		count_lines(const char *source);
	int		con_x(int n);
	int		con_y(int n);
	void	delete_block(int ncol, int nrow, int nlen);
	void	lprintf(int sy, const char *fmt, ...);
	void	cprintf(int sy, const char *fmt, ...);
	void	rprintf(int sy, const char *fmt, ...);
	void	footer(int sy, const char *fmt, ...);
	void	header(int sy, const char *fmt, ...);
	char	*dlq_cp(char *dest, int maxcp, const char *delims, char *p);
	char	*colorize(const char *src, int ln);

	// file i/o
	int		read_file(const char *f_name, bool warn = false, bool fix_first_line = false);
	int		save_file(const char *f_new_name, int start, int end, bool warn = false);

    // UNDO/REDO related functions
	void	free_undo_node(undo_node_t *node);
    void	free_undo_table(int warea);
    void	undo_advance_header(int warea);
	undo_node_t *undo_add_move(int warea, int old_col, int old_row);
	undo_node_t *undo_add_line(int warea, int old_col, int old_row, const char *old_text);
	undo_node_t *undo_add_block(int warea, int old_col, int old_row, const char *block);
	undo_node_t *undo_add(int warea, undo_node_t *old_node);
	undo_node_t *undo_add_rmblock(int warea, int old_col, int old_row, const char *block);
	undo_node_t *undo_add_rmline(int warea, int old_col, int old_row, int count = 1);
	undo_node_t *undo_add_clip(int warea);

	void	select_from_list_draw_node(int fs, int i, const char *src, bool sel);

protected:	
	void	goto_ypos(int ypos);
	void	goto_xpos(int xpos);

    bool	has_selection();
	void	cancel_selection();
    char	*get_selection()		{ return get_text(0); }

    // undo 
    // note: undo supported from any user_xxx function, I've not change the rest yet
	void	user_linecopy();
	void	user_linecut(bool storeit=true);
    void	user_goto();
	void	user_undo();
	void	user_copy();
	void	user_delsel();
	void	user_help_word();

	// pseudo-terminal
	virtual void	clrscr()							{ }						// clear screen
	virtual void	clreol()							{ }						// clear to EOL
	virtual void	gotoxy(int x, int y)				{ }						// set cursor position
	virtual	void	getxy(int *x, int *y)				{ *x = *y = 0; }		// get cursor position
    virtual void	deleteln()							{ }						// delete the line at cursor and scroll up the rests
    virtual void	insertln()							{ }						// insert an empty line at cursor and scroll down the rests
	virtual void	printw(const char *fmt, ...)		{ }						// printf    
	virtual int		get_pref_ch()						{ return 0; }			// gets a key (similar to getch() but for extended keys uses > 0x100)
	virtual char	*input(const char *defval=NULL);							// get a string from console, returns a static string, str[0] = 0 for escape
	virtual int		scr_rows()							{ return 25; }			// returns the number of the rows of the window
	virtual int		scr_cols()							{ return 80; }			// returns the number of the columns of the window
	virtual void	beep()								{ }						// just produce a beep
	virtual void	addch(int ch)						{ }						// print a char at current position
	virtual void	refresh()							{ }						// flush screen
	virtual void	sys_exit()							{ }						// Alt+X
    virtual void	set_color(int color)				{ }
	virtual void	help_on_word(const char *keyword)	{ }

    //////////////////////////////////////////////////////////////////////
	// interface
public:	
    virtual void	menu()								{ }
	virtual	void	help()								{ }
	virtual void	exec_shell_cmd(const char *com)		{ }	
	virtual	void	status_line(const char *say);												// print this to status line
	virtual char*	ask(const char *prompt, char *buf, int size=256, const char *defval=NULL);	// asks for a string
	virtual	int		iask(const char *prompt, int default_value = 0);							// asks for an integer
	virtual	int		select_from_list(const char *title, const char *list, int count=-1, int defsel=0);	// display & selects from a list

    // notify
    virtual void	clip_copy()							{ }
    virtual	void	clip_paste()						{ }

public:
	TED();
	virtual ~TED();

    // normal commands
	const char	*get_curr_name()							{ return wareas[current_warea]._fname; }
	const char	*get_curr_title()							{ return basename(get_curr_name());	}
	void	prompt(const char *fmt, ...);

	// ...
	void	goto_line(int lnumber);
	int		primitive_key(int c);
	void	adjust_screen(bool mode);
	void	redraw_screen(bool brefresh = true);
	void	place_cursor();
	void	manage_key(int c);

	//
    void	load(const char *file, int warea=1);
	void	save(const char *file, int warea=1);
	void	save_to(const char *file, int warea=1);
    void	newfile(const char *file, const char *deftext = NULL, int warea=1);
    char	*filetext(int warea=1)						{ return get_text(warea); }

	char	*get_text(int warea=1, line *start=NULL, int lines=0);
	void	set_text(const char *str, int warea=1, line *start=NULL);

	void	activate(int warea=1);

    int		get_tab_size() const						{ return tab_size; }
	void	set_tab_size(int size=4) 					{ tab_size = size; }

	//
	bool	add_stx(const char *file, const char *ext = NULL, const char *sign = NULL);
	void	scan_stx(int warea = 1);
	void	set_stx(int warea=1, int code=0);
	bool	stx_search(stx_t *node, const char *source,  bool caseless = true);

    // returns the current column
    int		col() const
			{ return real_col(wareas[current_warea]._x, wareas[current_warea]._curr->l); }

	// returns the columns end position    
    int		col_end() const						
			    { return real_col(strlen(wareas[current_warea]._curr->l)-1, wareas[current_warea]._curr->l); }

    // returns the current line number
    int		row() const
			{ return wareas[current_warea]._line_num; }

    // returns the current working area (buffer/file) number
    int		area() const
			{ return current_warea; }

    // clipboard
	char	*get_clip();
	void	set_clip(const char *text);

	bool	modified(bool val, int warea=1);
	bool	modified(int warea=1);

    // sets the current position based on screen-coords (used by mouse-click)
    void	set_curpos(int x, int y)		{ goto_ypos(y); goto_xpos(x); }
	};

#endif
