/**
*	TED, 
*
*	Ted is an editor class.
*	+ normal undo (not line-undo)
*	+ multiples buffer
*	+ customized keys
*	+ customized syntax-highlight
*
*	Nicholas Christopoulos.
*
*	This code is an evolution of TED 2.0 (1990),
*	TED2 was created by Fernando Joaquim Ganhao Pereira.
*	Fernado (as far as I know) he is supports another editor, fted.
*
*	TODO:
*	--- a lot, but first I must remove all static variables	
*	--- rename any 'area' to 'buffer'
*	--- check fix undo
*/

#if defined(_Win32)
#include <windows.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#if defined(__BORLANDC__)
        #include <io.h>
        
        #define F_OK    0
        #define X_OK    1
        #define W_OK    2
        #define R_OK    4
        #define RW_OK   6
        
        #define strcasecmp(a,b)         stricmp((a),(b))
        #define strncasecmp(a,b)        strnicmp((a),(b))
        
		#include <dir.h>
		#include <dirent.h>
		#include <sys/stat.h>
#else
        #include <unistd.h>
		#include <dirent.h>
		#include <sys/time.h>
		#include <sys/stat.h>
#endif
#include "ted.h"

#if defined(__BORLANDC__)
#pragma warn -8012
#endif

#if defined(_UnixOS)
	#define DEFVAR		"~/TED_DEFS"
	#define CLIPVAR		"~/.clipboard"
#else			   
	#define DEFVAR		"C:\\TED_DEFS"
	#define CLIPVAR		"C:\\TED_CLIPFILE"
#endif

#define	NONAME			"---"

#define SCREEN_LINES	scr_rows()
#define SCREEN_COLS 	scr_cols()
#define LAT_SHIFT		((SCREEN_COLS/5)+tab_size-(SCREEN_COLS/5)%tab_size)

#define CSZ(c,l)		(((c)== '\t') ? tab_size-(l)%tab_size : ((unsigned char)(c)>= ' ') ? 1 : 2)
#define EOL(line)		((int) strlen(line->l)-1)
#define ADJUST_X()		( x = act_x > EOL(curr) ? EOL( curr ) : act_x )
#define is_print( c ) 	( (((c) == 9) || ((c) > 31)) && (c) < 256 )
                                                                     
#define head		wareas[current_warea]._head
#define curr		wareas[current_warea]._curr
#define screen_top	wareas[current_warea]._screen_top
#define changed		wareas[current_warea]._changed
#define line_num	wareas[current_warea]._line_num
#define buffer_size	wareas[current_warea]._buffer_size
#define x			wareas[current_warea]._x
#define y			wareas[current_warea]._y
#define	cur_row		(wareas[current_warea]._line_num-1)
#define act_x		wareas[current_warea]._act_x
#define cur_col		(wareas[current_warea]._x)	// ???
#define fname		wareas[current_warea]._fname
#define l_mark		wareas[current_warea]._l_mark
#define x_mark		wareas[current_warea]._x_mark
#define sx_mark		wareas[current_warea]._sx_mark
#define sl_mark		wareas[current_warea]._sl_mark
#define hsyntax		wareas[current_warea]._hsyntax

#define	refresh_current_line()	redraw_line(y, curr->l, line_num)

// set undo-move buffer
#define	undo_move()				undo_add_move(current_warea, x, line_num);
// set undo-line buffer
#define	undo_line()				undo_add_line(current_warea, x, line_num, curr->l);
// set undo-clipboard 
#define	undo_clip()				undo_add_clip(current_warea);

typedef char*	char_p;

/**
*	constructor
*/
TED::TED()
{
    int		i;

    tab_size = 4;
	insert = true;
	indent = true;
	sensit = false;
	direction = true;			// search direction (true = forward)
    jump_scroll = scr_rows()-2;	// scroll 
    
	first_col = 0; 
	screen_lines = scr_rows();
	screen_cols = scr_cols();
	lat_shift = LAT_SHIFT;

	*clip_line = '\0';
	single_sel = 1;
	n_alias = 0;
	n_kdefs = 0;
	macro_changed = 0;
	def_macro_ndx = 0;
	alias_list = NULL;
	key_defs = NULL;
    last_page = NULL;
    last = NULL;
    last_key = 0;
	stx_table = NULL;
	stx_count = 0;
	sel_style = 0;
	in_editor_scr = true;

	char_xscroll = '>';
	char_empty = '~';

    strcpy(last_message, "");
    strcpy(last_usermsg, "");
    strcpy(pattern, " ");

	current_warea = 0;
	wareas[0]._head.l = NULL;
	wareas[0]._head.prev = &head;
	wareas[0]._head.next = &head;
	wareas[0]._curr = &head;
	wareas[0]._screen_top = &head;
	wareas[0]._line_num = 0;
	wareas[0]._buffer_size = 0;
	wareas[0]._changed = 0;
	wareas[0]._x = 0;
	wareas[0]._y = 0;
	wareas[0]._act_x = 0;
	wareas[0]._hsyntax = 0;
	strcpy(wareas[0]._fname, NONAME);

    // clear bookmarks
    for ( i = 0; i < 10; i ++ )
		wareas[0]._l_mark[i] /* line */ = wareas[0]._x_mark[i] /* column */ = 0;

	// selection marks        
	wareas[0]._sx_mark = 0;	// column
	wareas[0]._sl_mark = 0;	// line

    // build app_dir
    // build CLIPVAR
    
    clrscr();

    for( i = 1; i < TED_MAX_WAREAS; ++i )	{
    	wareas[i] = wareas[0];
		wareas[i]._head.prev  = &wareas[i]._head;
		wareas[i]._head.next  = &wareas[i]._head;
		wareas[i]._curr       = &wareas[i]._head;
		wareas[i]._screen_top = &wareas[i]._head;
		strcpy(wareas[i]._fname, NONAME);
        }

	// area 0 = clipboard
    current_warea = 0;
    insert_newline(&head, "\n");
	#if defined(_DOS)
	sprintf(fname, "%s", "clip.tmp");
    #elif defined(_WinBCB)
    {
    	HANDLE	happ;
        char	appname[1024], *p;
        
	    happ = GetModuleHandle(NULL);
		GetModuleFileName(happ, appname, 1024);
        p = strrchr(appname, '\\');
        *p = '\0';
		sprintf(fname, "%s\\%s", appname, "clipbrd.sbi");
	}
	#else
	sprintf(fname, "%s/%s", getenv("HOME"), ".clipboard");
	#endif
    curr = screen_top = head.next;
    line_num = 1;
	single_sel = 0;

    // init other areas
    for( i = 0; i < TED_MAX_WAREAS; ++i ) {
		//	building undo-tables
	    wareas[i].undo_head  = wareas[i].undo_tail = 0;
		wareas[i].undo_table = (undo_node_t*) malloc(sizeof(undo_node_t) * TED_UNDO_NODES);
        memset(wareas[i].undo_table, 0, sizeof(undo_node_t) * TED_UNDO_NODES);

       	if	( i > 0 )	{
			current_warea = i;
	       	insert_newline(&head, "\n");
			curr = screen_top = head.next;
			line_num = 1;
            }
    	}

    //
    select_warea(1);
}

/**
*	destructor
*/
TED::~TED()
{
	int		i;

	set_color(TED_TEXT_COLOR);

    for ( i = 0; i < TED_MAX_WAREAS; i ++ )		{
    	erase_working_area(i);
        free_undo_table(i);
        }

	for ( i = 0; i < stx_count; i ++ )	{
		stx_t	*node;
		node = &stx_table[i];
		if	( node->ext )
			free(node->ext);
		if	( node->sign )
			free(node->sign);
		if	( node->csv )
			free(node->csv);
		if	( node->index )
			free(node->index);
		}
	if	( stx_table )
		free(stx_table);
}

/**
*	undo: free node
*/
void	TED::free_undo_node(undo_node_t *node)
{
    switch ( node->code )	{
    case	undo_line:
    case	undo_block:
    case	undo_clipboard:
//    case	undo_rmblock:
        if	( node->ptr )
    	    free(node->ptr);
        break;
    case	undo_big_block:
        remove(node->ptr);
        if	( node->ptr )
	        free(node->ptr);
        break;
    default:
        ;
        }
	node->code = undo_null;        
    node->count = 0;
    node->ptr = NULL;
}

/**
*	free undo table
*/
void	TED::free_undo_table(int warea)
{
	while ( wareas[warea].undo_head != wareas[warea].undo_tail )	{
    	undo_node_t	*node = &wareas[warea].undo_table[wareas[warea].undo_tail];
        free_undo_node(node);
		wareas[warea].undo_tail ++;
    	}
        
	free(wareas[warea].undo_table); 
           
	wareas[warea].undo_tail = wareas[warea].undo_head = 0;
	wareas[warea].undo_table = NULL;
}

/**
*	undo: advance undo head-ptr
*/
void	TED::undo_advance_header(int warea)
{
	// advance head ptr
	wareas[warea].undo_head	++;
    if	( wareas[warea].undo_head == TED_UNDO_NODES )	
    	wareas[warea].undo_head = 0;
        
	if	( wareas[warea].undo_tail == wareas[warea].undo_head )	{
    	// the tail points to current node but the buffer is full (not empty)
        // we advance tail too, so, tails always become one node forward
		wareas[warea].undo_tail ++;
	    if	( wareas[warea].undo_tail == TED_UNDO_NODES )
			wareas[warea].undo_tail = 0;
        }
}

/**
*	undo: adds a movement change
*/
TED::undo_node_t *TED::undo_add_move(int warea, int old_col, int old_row)
{
	undo_node_t	*node = &wareas[warea].undo_table[wareas[warea].undo_head];

    node->code = undo_cursor;
    node->col  = old_col;
    node->row  = old_row;
	undo_advance_header(warea);
    return node;
}

/**
*	undo: adds a normal change
*/
TED::undo_node_t *TED::undo_add_line(int warea, int old_col, int old_row, const char *old_text)
{
	undo_node_t	*node = &wareas[warea].undo_table[wareas[warea].undo_head];

    node->code = undo_line;
    node->col  = old_col;
    node->row  = old_row;
    node->ptr  = strdup(old_text);
	undo_advance_header(warea);
    return node;
}

/**
*	undo: add a 'restore clipboard' node
*/
TED::undo_node_t *TED::undo_add_clip(int warea)
{
	if	( warea != 0 )	{
		undo_node_t	*node = &wareas[warea].undo_table[wareas[warea].undo_head];

        node->code = undo_clipboard;
        node->col  = node->row  = -1;
        node->ptr  = get_clip();
        undo_advance_header(warea);
        return node;
        }
	return NULL;        
}

/**
*	undo: adds a block of lines
*/
TED::undo_node_t *TED::undo_add_block(int warea, int old_col, int old_row, const char *block)
{
	undo_node_t	*node = &wareas[warea].undo_table[wareas[warea].undo_head];

    node->code = undo_block;
    node->col  = old_col;
    node->row  = old_row;
    node->ptr  = strdup(block);
	undo_advance_header(warea);
    return node;
}

/**
*	undo: adds a block of lines to be removed
*/
TED::undo_node_t *TED::undo_add_rmblock(int warea, int old_col, int old_row, const char *block)
{
	undo_node_t	*node = &wareas[warea].undo_table[wareas[warea].undo_head];

    node->code = undo_rmblock;
    node->col  = old_col;
    node->row  = old_row;
    node->len  = strlen(block);
	undo_advance_header(warea);
    return node;
}

/**
*	undo: adds a block of lines to be removed
*/
TED::undo_node_t *TED::undo_add_rmline(int warea, int old_col, int old_row, int count)
{
	undo_node_t	*node = &wareas[warea].undo_table[wareas[warea].undo_head];

    node->code = undo_rmline;
    node->col  = old_col;
    node->row  = old_row;
    node->len  = count;
	undo_advance_header(warea);
    return node;
}

/**
*	undo: adds a node
*/
TED::undo_node_t *TED::undo_add(int warea, undo_node_t *old_node)
{
	undo_node_t	*node = &wareas[warea].undo_table[wareas[warea].undo_head];

    node->code = old_node->code;
    node->col  = old_node->col;
    node->row  = old_node->row;
    if	( old_node->ptr )
	    node->ptr = strdup(old_node->ptr);
	undo_advance_header(warea);
    return node;
}

/**
*	undo: undo!
*/
void	TED::user_undo()
{
	int		warea = current_warea;
	undo_node_t	*node;
    
	if	( wareas[warea].undo_head != wareas[warea].undo_tail )	{	// undo-table is not empty
        // decrease undo pointer
		wareas[warea].undo_head --;
	    if	( wareas[warea].undo_head < 0 )
			wareas[warea].undo_head = TED_UNDO_NODES - 1;
            
		node = &wareas[warea].undo_table[wareas[warea].undo_head];

        // add a redo :)
        // ...

        // undo
        if ( node->code != undo_null )	{
            switch ( node->code )	{
            case undo_clipboard:
            	set_clip(node->ptr);
                break;
            case undo_line:
            	free(curr->l);
                curr->l = node->ptr;
                node->ptr = NULL;
				redraw_line(y, curr->l, line_num);
                break;
            case undo_rmline:
				{
					int		i;
					line	*next;

                    goto_line(node->row);

					for ( i = 0; i < node->len; i ++ )	{
						next = curr->next;
						delete_line(curr);
						curr = next;
						}
					
					curr = curr->prev;
	            	adjust_screen(true);
				}
                break;
            case undo_block:
            	set_text(node->ptr, current_warea, curr);
            	adjust_screen(true);
                break;
            case undo_big_block:
            	// TODO
            case undo_rmblock:
            	delete_block(node->col, node->row, node->len);
            	adjust_screen(true);
                break;
                }
                
        	// undo move
            if	( node->row != -1 && node->col != -1 )	{
                if	( line_num != node->row )	{
                    goto_line(node->row);
                    x = act_x = node->col;
                    }
                else	{
                    if	( x != node->col )
                        x = act_x = node->col;
                    }
                }

            // execute related nodes
            while ( node->count )	{
            	user_undo();
            	node->count --;
                }
                
			free_undo_node(node);
			prompt("undone");
            place_cursor();
        	}
        }
	else
    	prompt("nothing to undo");        
}

/**
*	return the number of the text-lines of a string
*/
int		TED::count_lines(const char *source)
{
	const char	*p;
    int		lines = 0;

    p = source;
    while ( (p = strchr(p, '\n')) != NULL )	{
    	p ++;
        lines ++;
    	}
	return lines;
}

/**
*	returns the text of the nth line
*/
char	*TED::text_ln(int n)
{
	line	*ptr;
    int		ln;

    if	( line_num < n )	{
    	// search forward
    	ln = line_num;
        ptr = curr;
	    while ( ptr != &head )	{
	    	if	( ln == n )
	        	return ptr->l;
	    	ptr = ptr->next;
	        ln ++;
	    	}
		}
    else	{
    	// search backward
    	ln = line_num;
        ptr = curr;
	    while ( ptr != &head )	{
	    	if	( ln == n )
	        	return ptr->l;
	    	ptr = ptr->prev;
	        ln --;
	    	}
		}

    // impossible
	return NULL;
}

/**
*	returns the screen's y of the nth text line 
*/
int		TED::con_y(int n)
{
	return (n - (line_num - y));
}

/**
*	returns the screen's x of the nth character of the current line 
*/
int		TED::con_x(int n)
{
	char	*str;

    str = text_ln(n);
    if	( str )
    	// WRONG! FIX IT!
		return real_col(x, str);
	return 0;
}

/**
*	returns a pointer of file points to the last directory separator + 1
*/
const char	*TED::basename(const char *file)
{
	const char *p;

   	#if defined(_Win32) || defined(_DOS)
	p = (const char *) strrchr(file, '\\');
    #else
	p = (const char *) strrchr(file, '/');
    #endif
	if	( p )
		return p+1;
	return file;
}

/**
*	Inserts a new line at pos
*/
TED::line*	TED::insert_newline(line *pos, char *str)
{
    line *new_line;

    new_line = (line *)    malloc(sizeof(line));

    new_line->l = strdup(str);
    if	( pos->next != &head )		// ndc
	    pos->next->prev = new_line;
    new_line->next = pos->next;
    new_line->prev = pos;
    pos->next = new_line;

	buffer_size ++;
    return new_line;
}

/**
*	delete the 'pos' line
*/
void	TED::delete_line(line *pos)
{
    if ( screen_top == pos )	
    	screen_top = pos->next;

    pos->next->prev = pos->prev;
    pos->prev->next = pos->next;
    
	buffer_size --;
    free(pos->l);
    free(pos);
}

/**
*/
void	TED::delete_block(int ncol, int nrow, int nlen)
{
	// we must remove node->len bytes from the position (node->col, node->row)
	// it is a multiline delete
	int		len = nlen;
	int		lsz;
	line	*ptr;
                    
	ptr = curr;
	if	( ncol )	{
		// start from somewhere in the line
		lsz = strlen(ptr->l) - ncol;
		ptr->l[ncol] = '\0';
        len -= lsz;
        ptr = ptr->next;
        }

	// 
	while ( ptr != &head )	{
		lsz = strlen(ptr->l);
		if	( lsz > len )	{
			// the last line (partial remove)
			strcpy(ptr->l, ptr->l+len);
			break;
			}
		else	{
			// delete line
                        
            if	( ptr == screen_top )	{
                screen_top = ptr->next;
                if	( screen_top == &head )
                    head.next = screen_top = ptr->prev;
                }
            if	( ptr == curr )	{
                curr = curr->next;
                if	( curr == &head )
                    curr = ptr->prev;
                }
                            
            ptr = ptr->next;
            delete_line(ptr->prev);
            }
        len -= lsz;
        }
}

/**
*	reads a file in to current area.
*
*	if erase_area is false the file will inserted to the current position;
*	otherwise the area will be erase before the file will be added.
*/
int		TED::read_file(const char *f_name, bool warn, bool erase_area)
{
    char	b[TED_LSZ];
    FILE	*fptr;
    line	*new_line, *ptr, *prev;

    if	( access(f_name, R_OK) != 0 )	{
       	prompt("cannot read '%s'", f_name);
        return 0;
        }
        
	if	( (fptr = fopen(f_name, "r")) == NULL )	{
		if ( warn )	
        	prompt("cannot read '%s'", f_name);
		return 0;
		}

	if	( erase_area )	{
	    ptr = head.next;
	    head.next = head.prev = &head;

        while ( ptr != &head )	{
        	prev = ptr;
            ptr  = ptr->next;
            free(prev->l);
            free(prev);
        	}

        //
        ptr = curr = screen_top = &head;
        line_num = 0;	// 1?
        buffer_size = 0;
	    curr = screen_top = &head;
        }
    else
    	ptr = curr;
        
    while ( fgets(b, TED_LSZ-1, fptr) )	{
		if	( b[strlen(b)-1] != '\n' )
			strcat(b, "\n");
            
		ptr = insert_newline(ptr, b);
    	}
    fclose(fptr);
    
	if	( erase_area )
    	changed = false;	// loading a file
	else
    	changed = true;		// merging a file

    if ( warn )	
    	prompt("'%s' loaded, %d lines", basename(f_name), buffer_size);
    return 1;
}

/**
*/
int		TED::save_file(const char *f_new_name, int start, int end, bool warn)
{
    FILE 	*fptr;
    line 	*ptr;
    char 	string[TED_LSZ];
    int 	i;
	char	*f_name;

	if	( !f_new_name )
		f_name = fname;
	else
		f_name = (char *) f_new_name;

    if ( start > end ) {
		i = start;
		start = end;
		end = i;
	    }

    fptr = fopen(f_name, "w");
    if ( fptr == NULL ) {
		if ( warn )	
			prompt("cannot write \'%s\'", f_name);
		return 0;
		}

    ptr = &head;
    i = 0;
    do	{
		ptr = ptr->next;
		++i;
		if ( i < start || i > end ) 
			continue;

		if	( ptr != &head )
			fputs( ptr->l, fptr );
		} while( ptr != &head );

	if	( warn )	
	    prompt("saved \'%s\', %d lines", f_name, end-start+1);

    fclose( fptr );
    return 1;
}

/**
*	moves (and makes visible) the editor to line_num
*/
void	TED::goto_line(int ln)
{
    int 	i = 0;

    if ( ln < 1 ) ln = 1;
    if ( ln > buffer_size ) ln = buffer_size;

    curr = &head;
	while( i < ln ) {
		curr = curr->next;
		if( curr == &head ) break;
		++i;
	    }
	line_num = (ln > 0) ? ln : buffer_size;
    adjust_screen(false);
}

/**
*	returns the line's ln line-ptr 
*/
TED::line	*TED::get_line_node(int ln)
{
	line	*ptr = head.next;
	int		n = 1;

    while ( ptr != &head )	{
    	if	( n == ln )
        	return ptr;
    	ptr = ptr->next;
        n ++;
    	}
	return NULL;        
}

/**
*	sets the current editors-line pointer based on the ypos of the screen
*
*	used by mouse click
*/
void	TED::goto_ypos(int ypos)
{
	int		dy = ypos - y;

    goto_line(line_num + dy);
    adjust_screen(true);
    place_cursor();
    refresh();
}

/**
*	sets the current editor's line, x posm based on the x-pos of the screen
*
*	used by mouse click
*/
void	TED::goto_xpos(int xpos)
{
	int		col = 0;
    char	*p = curr->l;
    int		vxpos = xpos + first_col;
    int		px = 0;
    
	while ( col < vxpos && *p != '\n' ) {
	    col += CSZ(*p, col);
	    p ++;
        px ++;
		}
	x = act_x = px;        

	place_cursor();	
    refresh();
}

/**
*/
void	TED::adjust_screen(bool mode)
{
    line 	*ptr = screen_top;
    int 	i;

    if	( ptr )	{
        if	( mode )	{
            // ndc update window size
            screen_lines = scr_rows();
            screen_cols  = scr_cols();
            lat_shift = LAT_SHIFT;
            jump_scroll = scr_rows()-2;
            }

        ADJUST_X();
    
        for( i = 0; i < screen_lines; ++i ) {
            if( ptr == &head )
                break;
            if( ptr == curr ) {
                y = i;
                if( mode ) 
                    redraw_screen();
                return;
                }
            ptr = ptr->next;
            }

		if	( curr )	{
            screen_top = curr;
            for( i = 0; i < screen_lines / 2; ++i ) {
                if( screen_top->prev == &head ) 
                    break;
                screen_top = screen_top->prev;
                }
        	}

        y = i;
        redraw_screen();        
		}
}

/**
*	full repaint
*
*	@param brefresh true to flush data
*/
void	TED::redraw_screen(bool brefresh)
{
    line 	*ptr = screen_top;
    int 	i, j, df;
    int		typo_line;

    for( i = df = 0; i < screen_lines; i ++, df ++ ) {
		if  ( ptr == &head )   	break;
    	if	( ptr == curr )    	break;
        ptr = ptr->next;
    	}
        
	if	( screen_top == &head )
    	return;
    
 	ptr = screen_top;
    for ( i = 0; i < screen_lines; ++i ) {
		if( ptr == &head ) break;

		typo_line = i+(line_num-df);	// current printing line
        
        // print text
		redraw_line(i, ptr->l, typo_line);

        // bookmark second style
       	for ( j = 0; j < 10; j ++ )	{
           	if	( l_mark[j] == typo_line )	{
                set_color(TED_BOOKMARK_COLOR);
                gotoxy(screen_cols-1, i);
                addch('0'+j);
                set_color(TED_TEXT_COLOR);
              	}
			}                

        // next
		ptr = ptr->next;
	    }
    while( i < screen_lines )
    	redraw_line(i++, "", -1);

	set_color(TED_STATUS_COLOR);
	status_line(last_message);
	set_color(TED_TEXT_COLOR);
	if	( brefresh )
	    refresh();
}

/**
*	return true if there is selected text
*
*	@return true if there is selected text
*/
bool	TED::has_selection()
{
	return (sl_mark != 0);
}

/**
*	cancel selection
*/
void	TED::cancel_selection()
{
	sl_mark = 0;
}

/**
*	colorize line (syntax highlight) 
*
*	@param src the source text
*	@param ln the drawing line (number)
*	@return a newly created buffer with the color attributes (VGA16)
*/
char	*TED::colorize(const char *src, int ln)
{
	char	*p, *ret, *r, *t;
	bool	dq = false;
	bool	rm = false;
	char	key[33], last_ch = 0;
	int		remain = 0, remain_color, cpos;
	stx_t	*stx;

	ret = strdup(src);
	memset(ret, TED_TEXT_COLOR, strlen(src));
	p = (char *) src;
	r = (char *) ret;

	while ( *p )	{

		// character position in memory
		cpos = (int) (p - src);

		if	( hsyntax )	{	// if we have syntax-table

			stx = &stx_table[hsyntax-1];

			if ( remain )	{
				// there are characters to be drawed with the last color
				*r = remain_color;
				remain --;
				}
			else if ( rm )
				// we are on remark section
				*r = TED_REMARK_COLOR;
			else if	( *p == stx->strq_ch )	{
				// string start/end
				*r = TED_STRING_COLOR;
				dq = !dq;
				}
			else if ( dq )
				// we are on a string
				*r = TED_STRING_COLOR;
			else	{

				if	( *p == stx->rem_sym1[0] || *p == stx->rem_sym2[0] )	{
					// check if it is remark
					*r = TED_REMARK_COLOR;
					rm = true;
					}
				else if	( isalpha(*p) && !isalpha(last_ch) )	{
					// check keyword
					strncpy(key, p, 32);
					key[32] = '\0';
					t = key;
					while ( *t )	{
						if	( !isalnum(*t) )	{
							*t = '\0';
							break;
							}
						t ++;
						}

					if	( strcasecmp(key, stx->rem_sym1) == 0 || strcasecmp(key, stx->rem_sym2) == 0 || strcasecmp(key, stx->rem_sym3) == 0 )	{
						// remark keyword
						rm = true;
						*r = TED_REMARK_COLOR;
						}
					else if ( stx_search(stx, key) )	{
						// command
						*r = remain_color = TED_COMMAND_COLOR;
						remain = strlen(key) - 1;
						}
					else	// no keyword
						*r = TED_TEXT_COLOR;
					}
				else	// no remarks, no keyword
					*r = TED_TEXT_COLOR;
				}
			}

		// selected
        if	( sl_mark != 0 )	{
			// TODO: check selected
			bool	sel = false;

			// note: line_num = current line,  sl_mark/sx_mark = selection mark (line/x)

			if	( sl_mark < ln && ln < line_num )		// we are located between sl_marx and current line
				sel = true;
			else if	( line_num < ln && ln < sl_mark )	// we are located between current line and sl_mark
				sel = true;
			else if ( sl_mark == ln && ln == line_num )	{	// selection starts/ends in this line
				if	( sel_style == 0 )	{	// normal
					if	( sx_mark < x )
						sel = (sx_mark <= cpos && cpos <= x);
					else
						sel = (x <= cpos && cpos <= sx_mark);
					}
				else	// line selection
					sel = true;
				}
			else if ( sl_mark == ln && line_num < ln )	{		// selection starts at this line but the current line is up
				if	( sel_style == 0 )	// normal
					sel = (sx_mark >= cpos);
				else	// line selection
					sel = true;
				}
			else if ( sl_mark == ln && line_num > ln ) {		// selection starts at this line but the current line is down
				if	( sel_style == 0 )	// normal
					sel = (sx_mark <= cpos);
				else	// line selection
					sel = true;
				}
			else if ( line_num == ln && sl_mark < ln )	{		// we are in the current line, the selection is up
				if	( sel_style == 0 )	// normal
					sel = (x >= cpos);
				else	// line selection
					sel = true;
				}
			else if ( line_num == ln && sl_mark > ln )	{		// we are in the current line, the selection is down
				if	( sel_style == 0 )	// normal
					sel = (x <= cpos);
				else	// line selection
					sel = true;
				}

			// setup the color
			if	( sel )	// highlight (selected)
				*r = remain_color = *r | 0x70;
			else	// low (unselected)
				*r = remain_color = *r & 0x1F;
			}

		last_ch = *p;
		p ++;
		r ++;
		}

	return ret;
}

/**
*	draws a text line
*
*	@param yl screen y
*	@param src the text
*	@param r_line *estimated* line number
*/
void	TED::redraw_line(int yl, char *src, int r_line)
{
	int 	stc_col;	/* it was static... why ? */
	int		last_color = 0;
    char	*cptr, *attr, cur_attr, *ptr;
    int		cpos = 0;

	if	( yl < 0 || yl >= screen_lines ) 
    	return;
    
	attr = colorize(src, r_line);
	ptr  = src;
	cptr = ptr;
        
    stc_col = 0;
	gotoxy(0, yl);

    // starting shift ?
    if ( *ptr ) {
		while ( stc_col < first_col && *ptr != '\n' ) {
		    stc_col += CSZ(*ptr, stc_col);
		    ++ ptr;
			}

		stc_col -= first_col;
            
		if( stc_col > 0 ) {
		    clreol();
		    gotoxy(stc_col, yl);
			}

        //
        while ( 1 ) {
            cpos = ptr - cptr;
			cur_attr = attr[cpos];
			if	( cur_attr != last_color )	{
				set_color(cur_attr);
				last_color = cur_attr;
				}

            //
            if ( stc_col >= screen_cols ) {
                if( ptr[1] == '\0' ) 
                    break;
                
                set_color(TED_XSCROLL_COLOR);
                gotoxy(screen_cols-1, yl);
                addch(char_xscroll);
                set_color(TED_TEXT_COLOR);
				free(attr);
                return;
                }

            // draw control character        
            if( (unsigned char) *ptr < ' ' ) {
                if( *ptr == '\t' ) {
					int		nc, i;

					nc = tab_size-stc_col%tab_size;
					for ( i = 0; i < nc; i ++, stc_col ++ )
						addch(' ');
                    }
                else {
                    if( ptr[1] == '\0' ) 
                        break;
                    addch( '^' );
                    addch( 'A'+*ptr-1 );
                    stc_col += 2;
                    }
                }
            else {
                // normal character
                addch( *ptr );
                ++stc_col;
                }
            
            ++ptr;
            }
    	}
	else {
    	// empty 
		set_color(TED_EMPTY_LINE_COLOR);
	    addch(char_empty);
		}

   	//  colors, reset
	set_color(TED_TEXT_COLOR);
    
    if ( stc_col < screen_cols )
    	clreol();
	free(attr);
}


/**
*	place cursor to correct position
*/
void	TED::place_cursor()
{
	int		col;

	if	( in_editor_scr )	{
		col = real_col(x, curr->l);
	    prompt("");
	    if ( lateral_adjust(col) ) 
	    	redraw_screen();
		gotoxy(col - first_col, y);
	    refresh();
		}
	else
	    refresh();
}

/**
*	returns the real screen column
*
*	@param col is the columnn in string
*	@param str is the source string
*/
int		TED::real_col(int col, const char *str) const
{
	int		i,j=0;

    for ( i=0; i<col; ++i ) {
		j += CSZ( *str, j );
		++str;
	    }
    return j;
}

/**
*/
bool	TED::lateral_adjust(int col)
{
    if ( col < first_col || col - first_col >= screen_cols ) {
		while ( col - first_col >= screen_cols ) first_col += lat_shift;
		while ( col < first_col ) first_col -= lat_shift;
		return true;
	    }
    return false;
}

/**
*	user: go to line
*/
void	TED::user_goto()
{
	int		n;
	char	*s;

	n = iask("go to line:");
    if	( n > 0 && n <= buffer_size )	{
    	undo_add_move(current_warea, x, line_num);
		goto_line(n);
        }
	prompt("");
}

/**
*	execute 'keys'
*/
int		TED::primitive_key(int c)
{
    repeating_key_2 = (repeating_key && (last_key == c));
    repeating_key = (last_key == c);
    last_key = c;

    // set undo-line buffer (ted)
    if	( curr != last )	{
		strcpy(undo_buffer, curr->l);
		last = curr;
	    }
        
	// fast insert
	if ( (c) >= 32 && c <= 255 && c != 127 ) {
    	// set undo-line buffer (ndc)
	    undo_add_line(current_warea, x, line_num, curr->l);

		if ( insert )
			insert_char(c);
		else
			overwrite(c);
		return 0;
    	}

	// execute keycode
    switch ( c )	{
	case TEDK_DEFKEY:
		define_key(); 
		break;
	case TEDK_INSOVR:
		insert = !insert;
		prompt(insert ? "insert mode" : "overwrite mode" );
		break;
	case TEDK_INDENT:
		indent = !indent;
		prompt( indent ? "auto-indent = on" : "auto-indent = off" ); 
		break;
	case TEDK_SENSIT:
		sensit = !sensit;
		prompt(sensit ? "case sensitivity = on" : "case sensitivity = off");
		break;
	case TEDK_CHCASE:
		change_case(); 
		break;
	case TEDK_MATCH:
    	undo_move();
		match();
		break;
	case TEDK_DELCHAR:
		delete_char();				// undo inside
		break;
	case TEDK_DELSEL:
		user_delsel();
		break;
	case TEDK_LINECOPY:
    	user_linecopy();
		break;
	case TEDK_LINECUT:
    	user_linecut();
		break;
	case TEDK_CUTSEL:
		if	( !has_selection() )
			user_linecut();
		else {
        	int	sx = sx_mark, sl = sl_mark;
            
			user_copy();
            sx_mark = sx;
            sl_mark = sl;
			user_delsel();
	        }
    	cancel_selection();
		break;
	case TEDK_COPYSEL:
		if	( !has_selection() )
			user_linecopy();
		else
			user_copy();
    	cancel_selection();
		break;
	case TEDK_PASTE:
		paste_buffer();
		break;
	case TEDK_SELMARK:
		if	( !has_selection() )	{
			sel_style = 0;	// normal
			selmark();
			}
		else	{
	    	cancel_selection();
			redraw_screen();
			}
		break;
	case TEDK_LINESELMARK:
		if	( !has_selection() )	{
			sel_style = 1;	// line
			selmark();
			}
		else	{
	    	cancel_selection();
			redraw_screen();
			}
		break;
	case TEDK_GOTO:
    	user_goto();
		break;
	case TEDK_SELCANCEL:
    	cancel_selection();
        redraw_screen();
		break;
	case TEDK_MARK0:
		mark(0);
		break;
	case TEDK_MARK1:
		mark(1);
		break;
	case TEDK_MARK2:
		mark(2);
		break;
	case TEDK_MARK3:
		mark(3);
		break;
	case TEDK_MARK4:
		mark(4);
		break;
	case TEDK_MARK5:
		mark(5);
		break;
	case TEDK_MARK6:
		mark(6);
		break;
	case TEDK_MARK7:
		mark(7);
		break;
	case TEDK_MARK8:
		mark(8);
		break;
	case TEDK_MARK9:
		mark(9);
		break;
	case TEDK_GMARK:
    	undo_move();
		recall_mark();
		break;
	case TEDK_SEARCH:
    	undo_move();
		search();
		break;
	case TEDK_FPREV:
		direction = !direction;
	case TEDK_FNEXT:
    	undo_move();
		find_next();
		break;
	case TEDK_REPEAT:
		repeat();
		break;
	case TEDK_SUBST:
		substitute();
		break;
	case TEDK_REFRESH:
       	adjust_screen(true);
//		redraw_screen();
		break;
	case TEDK_BOF:
		if ( cur_col == 0 && cur_row == 0 )	
			prompt("bof!");
		else	{
	    	undo_move();
			x = act_x = 0; 
			goto_line(1); 
			}
		break;
	case TEDK_PREVSCR:
		if ( cur_col == 0 && cur_row == 0 )
			prompt("bof!");
		else	{
	    	undo_move();
			prev_screen();
			}
		break;
	case TEDK_HOME:
		if ( cur_col == 0 && repeating_key_2 )	// HOME+HOME+HOME
			primitive_key(TEDK_BOF);
		else if ( cur_col == 0 && repeating_key )	{	// HOME+HOME
			primitive_key(TEDK_PREVSCR);
			last_key = TEDK_HOME;
			repeating_key = 1;
			}
		else {
	    	undo_move();
			home_col();
			}
		break;
	case TEDK_NEXTSCR:
		if	( line_num == buffer_size )
			prompt("eof!");
		else	{
	    	undo_move();
			next_screen();
			}
		break;
	case TEDK_EOF:
		if	( line_num == buffer_size )
			prompt("eof!");
		else	{
	    	undo_move();
			act_x = TED_LSZ;
			ADJUST_X();
			goto_line(buffer_size);
			}
		break;
	case TEDK_EOL:
		if	( x == EOL(curr) && repeating_key_2 )	// END+END+END
			primitive_key(TEDK_EOF);
		else if	( x == EOL(curr) && repeating_key )	{	// END+END
			primitive_key(TEDK_NEXTSCR);
			last_key = TEDK_EOL;
			repeating_key = 1;
			}
		else {
	    	undo_move();
			act_x = TED_LSZ;
			x = EOL( curr ); 
			}
		break;
	case TEDK_KILL:
    	user_linecut(false);
		break;
	case TEDK_WORD:
    	undo_move();
		advance_word();
		break;
	case TEDK_DELWORD:
    	undo_line();
		del_next_word();
		break;
	case TEDK_BWORD:
    	undo_move();
		back_word();
		break;
	case TEDK_DELBWORD:
    	undo_line();
		del_word();
		break;
	case TEDK_UNDO:
		user_undo();
		break;
	case TEDK_DELBOL:
    	undo_line();
		delBOL();
		break;
	case TEDK_DELEOL:
    	undo_line();
		delEOL();
		break;
	case TEDK_AGAIN:
		repeat_again();
		break;
	case TEDK_DOWN:
    	undo_move();
        move_down();
        break;
	case TEDK_LEFT:
    	undo_move();
        move_left();
        break;
	case TEDK_RIGHT:
    	undo_move();
        move_right();
        break;
	case TEDK_UP:
    	undo_move();
        move_up();
        break;
	case TEDK_RETURN:
		new_line();			// undo inside
		break;
	case TEDK_BSPACE:
		delete_backchar();	// undo handled inside
		break;
	case TEDK_MENU:
		menu();
		break;
	case TEDK_COMMAND:
		parse_command();
		break;
	case TEDK_HELP:
		help(); 
		break;
	case TEDK_CENTER:
		undo_move();
		screen_top = &head;
		curr = NULL;
		goto_line(line_num);
		break;
	case TEDK_NULL:	
		beep();
		break;
	case TEDK_NDC:	
    	ndc_console();
		break;
	case TEDK_BUFLIST:
		user_buflist();
		break;
	case TEDK_SYSEXIT:
    	exit_and_save(true);
		break;
	case TEDK_LOAD:
		{
			char	buf[256];
			
			ask("enter filename:", buf, 255, NULL);
			if	( strlen(buf) == 0 )
				break;

			#if defined(_SBIDE)
			if	( strchr(buf, '.') == NULL )
				strcat(buf, ".bas");
			#endif
			load(buf, current_warea);
		}
		break;
	case TEDK_SAVE:
	case TEDK_CHANGEFN:
		if	( strcmp(fname, NONAME) == 0 )	{
			char	buf[256];
			
			ask("enter filename:", buf, 255, NULL);
			if	( strlen(buf) == 0 )
				break;

			#if defined(_SBIDE)
			if	( strchr(buf, '.') == NULL )
				strcat(buf, ".bas");
			#endif
			if ( c == TEDK_CHANGEFN )	{
				if	( access(fname, F_OK) == 0 )	{
					if	( rename(fname, buf) == 0 ) {
						strcpy(fname, buf);
						save(buf, current_warea);
						prompt("file renamed and saved");
						}
					else
						prompt("rename failed");
					}
				else	{
					strcpy(fname, buf);
					save(buf, current_warea);
					prompt("file renamed and saved");
					}
				}
			else	{
				save(buf, current_warea);
				}
			}
		else
			save(NULL, current_warea);
		break;
	case TEDK_HELPWORD:
		user_help_word();
		break;
	case TEDK_NEXTBUF:
		current_warea ++;
		if	( current_warea >= TED_MAX_WAREAS )
			current_warea = 0;
		activate(current_warea);
		break;
	case TEDK_PREVBUF:
		current_warea --;
		if	( current_warea < 0 )
			current_warea = TED_MAX_WAREAS - 1;
		activate(current_warea);
		break;
	case TEDK_ALIAS:
		if ( subst_alias() ) 
			break;
	default:
		if ( !is_print(c) )
			beep();
		else	{
	    	// set undo-line buffer (ndc)
		    undo_add_line(current_warea, x, line_num, curr->l);
            
	         if ( insert )
				insert_char( c );
			else 
				overwrite( c );
        	}
		}

	// fix: redraw selection
	// todo: optimization
	if	( has_selection() )
//    	refresh_current_line();        
		redraw_screen();

    return 0;
}

/**
*	user: finds next
*/
void	TED::find_next()
{
    if ( find("", direction) )	{
    	prompt("found!");
    	adjust_screen(false);
        }
    else {
    	beep();
    	prompt("not found!");
        }
}

/**
*	user: search
*/
void	TED::search()
{
    char 	*str, buf[256];

  	str = ask("search for ([<|>]text) ?", buf, 256, pattern);
    if	( str[0] )	{
        if( *str == '>' || *str == '<' )
            direction = (*str++=='>');
            
        if ( find(str, direction) ) {
	    	prompt("found!");
            adjust_screen(false);
            }
        else	{
		   	beep();
            prompt( "not found!" );
            }
		}            
}

/**
*	search & update cursor
*
*	@param str is the string to search. if str[0]=0 then it uses the last string.
*	@param dir is the direction (true=forward, false=backward)
*	@return non-zero if found (and moves the cursor on the correct pos)
*/
int		TED::find(const char *str, bool dir)
{
    line 	*ptr;
    int		cl, new_x;

    if	( str[0] ) 
    	strcpy(pattern, str);
	if	( curr == &head )
    	return 0;        

    if ( dir ) {	
	    // search forward ?
        
    	// search in current line
		if( x < EOL(curr) && f_instr(pattern, curr->l+x+1) != -1 ) {
		    act_x = x += f_instr(pattern, curr->l+x+1)+1;
		    return 1;
			}

        // search in next lines
        cl = line_num + 1;
        ptr = curr->next;
        while ( ptr != &head )	{
			if	( (new_x = f_instr(pattern, ptr->l)) != -1 ) {
		        goto_line(cl);
				act_x = x = new_x;
            	return 1;	// found
                }
	        ptr = ptr->next;
	        cl ++;
            }
	    }
	else	{
	    // search backward ?

    	// search in current line
	    if( x > 0 && r_instr( pattern, curr->l, x-1 ) != -1 ) {
			act_x = x = r_instr( pattern, curr->l, x-1 );
			return 1;
		    }
    
        // search in previous lines
        cl = line_num - 1;
        ptr = curr->prev;
        while ( ptr != &head )	{
			if	( (new_x = r_instr(pattern, ptr->l, EOL(ptr))) != -1 ) {
		        goto_line(cl);
				act_x = x = new_x;
            	return 1;	// found
                }
	        ptr = ptr->prev;
	        cl --;
            }
		}
        
	return 0;	// not found
}

/**
*/
void	TED::substitute()
{
    static char pat[TED_LSZ] = "............";
    static char subs[TED_LSZ] = "............";
    int conf, i, j, c, n_subst;
    char *str;
    char string[TED_LSZ];

    prompt( "find >" );
    str = input();
    if( str[0] ) strcpy( pat, str );
    prompt( "substitute by >" );
    str = input();
    if( str[0] ) strcpy( subs, str );
    prompt( "from Line >" );
    i = get_val( input() );
    if( i < 1 || i > buffer_size ) return;
    prompt( "to Line >" );
    j = get_val( input() );
    if( j < 1 || j > buffer_size ) return;
    if( i > j ) { conf = i; i = j; j = conf; }
    prompt( "individual confirmations [y/n] ?" );
    conf = ( tolower(input()[0]) != 'n' );

    goto_line(i);
    n_subst = 0;

    while( find(pat, true) && line_num >= i && line_num <= j ) {
		i = line_num;
		if( conf ) {
		    adjust_screen(false);
		    prompt("confirm?" );
		    place_cursor();
		    c = tolower(get_pref_ch());
		    if( c == 'y' ) {
				subst_string( pat, subs );
				redraw_line( y, curr->l, line_num );
				++n_subst;
			    }
		    else 	{
		    	if ( c != 'n' ) 
		    		break;
				}
			}
		else {
		    subst_string( pat, subs );
		    ++n_subst;
			}
	    }

    adjust_screen(true);
    sprintf(string, "%03d Substitutions !", n_subst);
    prompt(string);
}

/**
*/
void	TED::subst_string(char *old, char *news)
{
    char aux[TED_LSZ];

    changed = true;
    strcpy( aux, curr->l+x+strlen(old) );
    curr->l = (char*)realloc(curr->l,strlen(curr->l)+strlen(news)-strlen(old)+1);
    curr->l[x] = '\0';
    x+=strlen(news);
    strcat( curr->l, news);
    strcat( curr->l, aux );
}

/**
*	user: page up
*/
void	TED::prev_screen()
{
    int i;

    for( i = 0; i < screen_lines; ++i ) {
		if( screen_top->prev == &head ) {
		    goto_line(1);
		    redraw_screen();
		    return;
			}
		screen_top = screen_top->prev;
		curr = curr->prev;
		--line_num;
	    }
    ADJUST_X();
    redraw_screen();
}

/**
*	user: page down
*/
void	TED::next_screen()
{
    int i;
    line *ptr;

    ptr = screen_top;

    for( i = 0; i < screen_lines; ++i ) {
		ptr = ptr->next;
		curr = curr->next;
		if( curr == &head )	{
			goto_line(buffer_size);
			return;
			}
		++line_num;
	    }

    screen_top = ptr;
    ADJUST_X();
    redraw_screen();
}

/**
*	user: begin-of-line
*/
void	TED::home_col()
{
    int i = 0;

    if ( indent )	{
		while ( curr->l[i] == ' ' || curr->l[i] == '\t' )
			i ++;
		}
    act_x = x = ( x > i ? i : 0 );
}

/**
*	user: delete to end-of-line
*/
void	TED::delEOL()
{
    strcpy( clip_line, curr->l+x );
    clip_line[strlen(clip_line)-1] = '\0';
    single_sel = true;

    strcpy( curr->l+x, "\n" );
    curr->l = (char *)realloc( curr->l, x + 2 );
    changed = true;
    redraw_line(y, curr->l, line_num);
}

/**
*	user: delete to begin-of-line
*/
void	TED::delBOL()
{
    int s = 0;
    if ( indent )
    	while ( curr->l[s] == ' ' || curr->l[s] == '\t' ) ++s;
    if( s >= x )
    	 s = 0;

    strncpy( clip_line, curr->l+s, x-s );
    clip_line[x-s-1] = '\0';
    single_sel = true;

    strcpy( curr->l+s, curr->l+x );
    curr->l = (char *)realloc( curr->l, strlen( curr->l ) + 1 );
    x = act_x = s;
    changed = true;
    redraw_line(y, curr->l, line_num);
}

/**
*	user: move one word forward
*/
void	TED::advance_word()
{
    if ( x >= EOL(curr) )	{
    	move_right();
        return;
        }

    if( isalnum(curr->l[x]) || curr->l[x] == '_' )
	while( x<EOL(curr) && (isalnum(curr->l[x])||curr->l[x] == '_')) ++x;
    else if( ispunct(curr->l[x]) )
	while( x<EOL(curr) && ispunct(curr->l[x]) ) ++x;
    else if( isspace(curr->l[x]) )
	while( x<EOL(curr) && isspace(curr->l[x]) ) ++x;
    else if( iscntrl(curr->l[x]) )
	while( x<EOL(curr) && iscntrl(curr->l[x]) ) ++x;
    else return;
    act_x = x;
}

/**
*	user: move one word backward
*/
void	TED::back_word()
{
    int i;

    if( !x )	{
    	move_left();
		return;
		}

    --x;

    if ( isalnum(curr->l[x]) || curr->l[x] == '_' )
		while( x>=0 && (isalnum(curr->l[x])||curr->l[x] == '_')) --x;
    else if( ispunct(curr->l[x]) )
		while( x>=0 && ispunct(curr->l[x]) ) --x;
    else if( isspace(curr->l[x]) )
		while( x>=0 && isspace(curr->l[x]) ) --x;
    else if( iscntrl(curr->l[x]) )
		while( x>=0 && iscntrl(curr->l[x]) ) --x;
    else
    	return;
    ++x;
}

/**
*	user: delete word backward
*/
void	TED::del_word()
{
    int i;

    if( !x ) return beep();
    i = x-1;

    if( isalnum(curr->l[i]) || curr->l[i] == '_' )
	while( i>=0 && (isalnum(curr->l[i])||curr->l[i] == '_')) --i;
    else if( ispunct(curr->l[i]) )
	while( i>=0 && ispunct(curr->l[i]) ) --i;
    else if( isspace(curr->l[i]) )
	while( i>=0 && isspace(curr->l[i]) ) --i;
    else if( iscntrl(curr->l[i]) )
	while( i>=0 && iscntrl(curr->l[i]) ) --i;
    else return;

    ++i;

    strncpy( clip_line, curr->l+i, x-i );
    clip_line[x-i] = '\0';
    single_sel = true;

    strcpy( curr->l+i, curr->l+x );
    curr->l = (char *)realloc( curr->l, strlen( curr->l ) + 1 );
    x = act_x = i;
    changed = true;
    redraw_line(y, curr->l, line_num);
}

/**
*	user: delete word forward
*/
void	TED::del_next_word()
{
    int i;

    if( x >= EOL(curr) ) return beep();
    i = x;

    if( isalnum(curr->l[i]) || curr->l[i] == '_' )
	while( i<EOL(curr) && (isalnum(curr->l[i])||curr->l[i] == '_')) ++i;
    else if( ispunct(curr->l[i]) )
	while( i<EOL(curr) && ispunct(curr->l[i]) ) ++i;
    else if( isspace(curr->l[i]) )
	while( i<EOL(curr) && isspace(curr->l[i]) ) ++i;
    else if( iscntrl(curr->l[i]) )
	while( i<EOL(curr) && iscntrl(curr->l[i]) ) ++i;
    else return;

    strncpy( clip_line, curr->l+x, i-x );
    clip_line[i-x] = '\0';
    single_sel = true;

    strcpy( curr->l+x, curr->l+i );
    curr->l = (char *)realloc( curr->l, strlen( curr->l ) + 1 );
    changed = true;
    redraw_line(y, curr->l, line_num);
}

/**
*	old undo... ignore it
*/
void	TED::undo()
{
    curr->l = (char *)realloc( curr->l, strlen( undo_buffer ) + 1 );
    strcpy( curr->l, undo_buffer );
    redraw_line(y, curr->l, line_num);
    ADJUST_X();
}

/**
*	user: move down
*/
int		TED::move_down()
{
    int retv = 1;

    if( curr->next == &head ) {
		beep();
		retv = 0;
	    }
    else {
		curr = curr->next;
		++line_num;
		if( y < screen_lines-1 ) 
			++y;
		else 
			scroll_up(curr->l);
	    }

    ADJUST_X();
    return retv;
}

/**
*	user: move left
*/
int		TED::move_left()
{
    if ( x )
    	act_x = --x;
    else   {
		act_x = x = TED_LSZ;
		if (! move_up())	{
		    act_x = x = 0;
		    return 0;
			};
    	}
    return 1;
}

/**
*	user: move right
*/
int		TED::move_right()
{
    if( x < EOL( curr ) ) act_x = ++x;
    else
    {
	act_x = x = 0;
	if(! move_down())
	{
	    x = act_x = EOL( curr );
	    return 0;
	}
    }
    return 1;
}

/**
*	user: move up
*/
int		TED::move_up()
{
    int retv = 1;

    if( curr->prev == &head ) {
	beep();
	retv = 0;
    }
    else {
	curr = curr->prev;
	--line_num;
	if( y ) --y;
	else scroll_down( curr->l );
    }
    ADJUST_X();
    return retv;
}

/**
*	the 'DELETE' key
*/
void	TED::delete_char()
{
    if( x < EOL(curr) )	{
		undo_line();
    	move_right();
	    delete_backchar();
		}
	else {
		undo_node_t	*unode;

		undo_move();															// keep position

		home_col();
		move_down();

		unode = delete_backchar();
		if	( unode )
			unode->count ++;
		}
}

/**
*	the 'BACKSPACE' key
*/
TED::undo_node_t	*TED::delete_backchar()
{
    line	*ptr;
    int 	i, max, final_x;
	undo_node_t	*unode;

    if ( x != 0 ) {
    	unode = undo_line();
		max = EOL( curr );
		for( i = --x; i <= max; ++i )
        	curr->l[i] = curr->l[i+1];
		act_x = x;
		redraw_line(y, curr->l, line_num);
	    }
    else if( curr->prev == &head ) {
		beep();
		return NULL;
	    }
    else {
		undo_move();															// keep position
		undo_add_line(current_warea, x, line_num-1, curr->l);					// keep the text of l1
		undo_add_line(current_warea, x, line_num, curr->prev->l);				// keep the text of l2
		unode = undo_add_block(current_warea, x, line_num-1, curr->prev->l);	// insert string
		unode->count = 3;														// run 3 more nodes

		curr = curr->prev;
		-- line_num;
		act_x = x = EOL(curr);
		curr->l[ strlen(curr->l) - 1] = '\0';
		curr->l = (char *)realloc( curr->l, x + strlen( curr->next->l ) + 2 );
		strcat( curr->l, curr->next->l );
		delete_line( curr->next );
		if( y ) {
		    deleteln();
		    redraw_line(--y, curr->l, line_num);	// check
		    ptr = curr;
		    for( i = y+1; i<screen_lines && ptr!=&head; ++i ) ptr = ptr->next;
		    if( ptr != &head ) 
            	redraw_line(screen_lines-1, ptr->l, line_num);
		    else 
            	redraw_line(screen_lines-1, "", -1);
			}
		else 
        	scroll_down(curr->l);
	    }

    changed = true;
	return unode;
}

/**
*/
void	TED::overwrite(char c)
{
    int 	i;

    if( x >= EOL( curr ) ) return insert_char( c );
    changed = true;
    curr->l[x] = c;
    redraw_line( y, curr->l, line_num);
    act_x = ++x;
}

/**
*/
void	TED::insert_char(char c)
{
	int		i;

    if( ( i = EOL( curr )+1 ) >= TED_LSZ ) return beep();
    changed = true;
    curr->l = (char *)realloc( curr->l, strlen( curr->l ) + 2 );
    for( ; i >= x; --i ) curr->l[i+1] = curr->l[i];
    curr->l[x] = c;
    redraw_line( y, curr->l, line_num);
    act_x = ++x;
}

/**
*/
void	TED::new_line()
{
    char news[TED_LSZ];
    line *n_line;
    int old_x;
	undo_node_t	*unode;

	undo_line();
	unode = undo_add_rmline(current_warea, x, line_num+1);
	unode->count = 1;

    old_x = x;
    act_x = x = 0;

    strcpy(news, "");
    if( indent ) {
	while( x < old_x && curr->l[x] == ' ' || curr->l[x] == '\t' ) {
	   news[x] = curr->l[x];
	   ++x;
	}
	news[act_x = x] = '\0';
    }
    strcat(news, curr->l+old_x );
    strcpy( curr->l+old_x, "\n" );
    curr->l = (char *)realloc( curr->l, old_x + 2 );

    clreol();
    if( y < screen_lines-1 ) {
		insertln();
		redraw_line( y++, curr->l, line_num);
	    }
    else scroll_up( curr->l );

    curr = insert_newline( curr, news);
    redraw_line( y, curr->l, line_num);
    ++line_num;
    changed = true;
}

/**
*/
void	TED::scroll_up(char *l)
{
    int i;
    for( i = 0; i < screen_lines / jump_scroll; ++i ) {
        if( screen_top->next == &head ) break;
        screen_top = screen_top->next;
    }
    y = screen_lines - ( i > 0 ? i : 0) ;
    redraw_screen();
}

/**
*/
void	TED::scroll_down(char *l)
{
    int i;
    for( i = 0; i < screen_lines / jump_scroll; ++i ) {
        if( screen_top->prev == &head ) break;
        screen_top = screen_top->prev;
    }
    y = i > 0 ? i-1 : 0;
    redraw_screen();
}

/**
*/
int		TED::get_val(char *str)
{
    int i = 0;

    switch (*str) {
	case '.' : i = line_num; ++str; break;
	case '$' : i = buffer_size; ++str; break;
	case 'x' : i = x; ++str; break;
	case 'y' : i = y; ++str; break;
	case 'e' : i = EOL( curr ); ++str; break;
	case 'c' : i = real_col(x, curr->l ); ++str; break;
	case 'm' : i = l_mark[0]; ++str; break;
	case 'M' : i = x_mark[0]; ++str; break;
	case 'b' : i = current_warea; ++str; break;
	case '/' : case '*' : return 0;
	case '+' : case '-' : break;
	default  : while( *str == ' ' || *str == '\t' ) ++str;
		   i = atoi( str );
		   while( *str >= '0' && *str <= '9' ) ++str;
    }
    while( *str == ' ' || *str == '\t' ) ++str;
    switch( *str ) {
	case '+' : i += get_val( ++str ); break;
        case '-' : i -= get_val( ++str ); break;
        case '*' : i *= get_val( ++str ); break;
        case '/' : i /= get_val( ++str );
    }
    return i;
}

/**
*	accept key codes
*/
int		*TED::get_cmd_str()
{
    static int b[TED_LSZ];
    int c;

    c = 0;
    while (1) {
		refresh();
		b[c] = get_pref_ch();
		if( b[c] == TEDK_ENTER || b[c] == TEDK_RETURN ) 
        	break;
		if( b[c] == TEDK_DELETE || b[c]==TEDK_BSPACE ) {
		    if( c ) { 
            	--c; 
                printw( "\b  \b\b" ); 
            	}
		    else 
            	beep();
		    continue;
			}
		if ( c < TED_LSZ -1 ) {
		    addch( is_print(b[c]) ? b[c] : '*' );
		    ++c;
			}
		else
        	beep();
	    }
        
    b[c] = 0;
    refresh();
    return b;
}

/**
*/
void	TED::repeat()
{
    static int last_cmd[TED_LSZ];
    static int n = -1;
    int *cmd;
    int i;

    prompt("how many times:" );
    i = get_val(input());
    if( i > 0 )
    	n = i;
    if ( n > 0 ) {
		prompt("command:");
		cmd = get_cmd_str();
		if ( cmd[0] )
			memcpy(last_cmd, cmd, TED_LSZ * sizeof(int));
		rep_command( n, last_cmd );
	    }
    prompt("");
}

/**
*/
void	TED::rep_command(int n, int *cmd)
{
    char number[20];
    int  num[20];
    int *ptr;
    int i,j;

    for( i = 1; i <= n; ++i ) {
		ptr = cmd;
		while( *ptr ) {
	    	place_cursor();
		    if( *ptr == TEDK_REFRESH|| *ptr == TEDK_REPEAT ||	*ptr == TEDK_DEFKEY ) {
				sprintf( number, "%d",
                		(*ptr==TEDK_REFRESH)? line_num :
				      	(*ptr==TEDK_DEFKEY)? i :
				      	real_col( x, curr->l)+1 );
				for( j = 0; number[j]; ++j ) num[j] = number[j];
			num[j] = 0;
			rep_command( 1, num );
		    }
	    else
	    	primitive_key(*ptr);
	    ++ptr;
		}
    }
}

/**
*/
int		TED::f_instr(char *st1, char *source)
{
	int		n, p=0;

    if( !source ) return -1;

    n = strlen( st1 );
    if( sensit ) while( *source ) {
	if( *st1 == *source && strncmp( source, st1, n ) == 0 ) return p;
	++source;
	++p;
    }
    else while( *source ) {
	if( (islower(*st1) ? toupper(*st1) : *st1) ==
	    (islower(*source) ? toupper(*source) : *source) &&
	    case_ins_strncmp( source, st1, n ) == 0 ) return p;
	++source;
	++p;
    }
    return -1;
}

/**
*/
int		TED::r_instr(char *st1, char *source, int start)
{
	int n, p=0;

    if( !source ) return -1;

    n = strlen( st1 );
    if( sensit ) {
	for( p = start; p >= 0; --p )
	if( *st1 == source[p] && strncmp( source+p, st1, n ) == 0 ) return p;
    }
    else {
	for( p = start; p >= 0; --p )
	if( (islower(*st1) ? toupper(*st1) : *st1) == 
	    (islower(source[p]) ? toupper(source[p]) : source[p]) &&
	    case_ins_strncmp( source+p, st1, n ) == 0 ) return p;
    }
    return -1;
}

/**
*/
int		TED::case_ins_strncmp(char *s1, char *s2, int n)
{
    while( ( islower(*s1) ? toupper(*s1) : *s1 ) ==
    	   ( islower(*s2) ? toupper(*s2) : *s2 ) && n>1 ) { --n; ++s1; ++s2; }
    return (islower(*s1)?toupper(*s1):*s1) - (islower(*s2)?toupper(*s2):*s2);
}

/**
*/
void	TED::parse_command()
{
    char string[TED_LSZ];
    char key[TED_LSZ];
    char *comm;
    int number;

    prompt("Command:");
    comm = input();

    switch( toupper( comm[0] ) ) {
	case 'Q' :
    	exit_and_save(false);
        return;
	case 'R' :
    	read_to_warea( comm );
		break;
	case 'W' :
    	write_warea( comm );
		break;
	case 'G' :
    	number = get_val( comm+1 );
		goto_line( number );
		break;
	case '!' :
    	exec_shell_cmd( comm+1 );
		break;
	case 'I' :
		user_buflist();
		break;
	case 'A' :
    	if(sscanf(comm,"%*s %10s %[^\n]", key, string)!=2)
        	break;
		create_alias( key, string );
		break;
	case 'S' :
    	if( strlen(comm)>2 ) save_defs( comm+2 );
		   else print_defs();
		   break;
	case 'L' : 
		read_defs((char *) (strlen(comm)>2 ? comm+2 : DEFVAR));
		break;
	case 'X' :
    	exit_and_save(true);
        return;
	case 'B':
    	number = get_val( comm+1 );
		select_warea( number );
		break;
	case 'H' : help();
		break;
	case 'P' : prompt("hit the new PREFIX key!" );
		   number = get_pref_ch();
		   if( number == TEDK_RETURN ) break;
		   pref_key = number;
		   sprintf( string, "new PREFIX key = [%03x HEX]!", pref_key );
		   prompt( string );
		   break;
	case 'D' : prompt( "delete everything ?" );
	 	   if( toupper(get_pref_ch()) == 'Y' )
			delete_range( 1, buffer_size );
		   break;
	default  : prompt("invalid command! (PREF<ESC>, h for help)" );
    }
    return;
}

/**
*	realy_save = save all and exit; or ask
*/
void	TED::exit_and_save(bool realy_save)
{
    int		c = 'y';
    int		i, ubc = 0;

    for ( i = 1; i < TED_MAX_WAREAS; i ++ )	{
		if ( wareas[i]._changed ) 
        	ubc ++;
		}

//	if ( !realy_save ) {
        if	( ubc == 1 )
            prompt("one buffer has not been saved. exit (y,n,w) ?");
        else if ( ubc )
            prompt("%d buffers have not been saved. exit (y,n,w) ?");
//		}
//    else
//    	c = 'w';	// save all and exit

//    if	( ubc && (!realy_save) )	{
    if	( ubc )	{
		char	*s;

		s = input();
		c = tolower(*s);
		}

	switch ( c ) {
    case	'y':	// dont save and exit   
	    finish();
    	break;
    case	'n':	// dont save, dont exit
    	break;
    case	'w':	// save all
	    for ( i = 1; i < TED_MAX_WAREAS; i ++ )	{
			if ( wareas[i]._changed ) 
				save(NULL, i);
			}
	    finish();
    	break;
        }
}

/**
*/
void	TED::finish()
{
	current_warea = 0;
    if( changed )	// && getenv(CLIPVAR)
		save_file(CLIPVAR, 2, buffer_size, false);
	sys_exit();
}

/**
*/
void	TED::create_alias(char *k, char *s)
{
    int i;

    for( i = 0; i< n_alias; ++i )
	if( !strcmp( k, alias_list[i].key )) break;
    if( i == n_alias ) {
	alias_list = (alias*) (n_alias ?
	    realloc( alias_list, sizeof(alias)*++n_alias) :
	    malloc( sizeof(alias) * ++n_alias ) );
	i = n_alias-1;
	strcpy( alias_list[i].key, k );
    }
    strncpy( alias_list[i].subs, s, 4 * TED_MKEYS );
}

/**
*/
char 	*TED::find_alias(char *key)
{
    int i;

    for( i = 0; i < n_alias; ++i )
	if( strcmp( key, alias_list[i].key )==0 ) return alias_list[i].subs;
    return NULL;
}

/**
*/
int		TED::subst_alias()
{
    int i;
    char key[TED_LSZ];
    char *s;

    if( !x ) return 0;
    i = ( x == 1 || isspace(curr->l[x-1]) ) ? x-1 : x-2;

    if( isalnum(curr->l[i]) || curr->l[i] == '_' )
	while( i>=0 && (isalnum(curr->l[i])||curr->l[i] == '_')) --i;
    else if( ispunct(curr->l[i]) )
	while( i>=0 && ispunct(curr->l[i]) ) --i;
    else if( isspace(curr->l[i]) )
	while( i>=0 && isspace(curr->l[i]) ) --i;
    else if( iscntrl(curr->l[i]) )
	while( i>=0 && iscntrl(curr->l[i]) ) --i;
    else return 0;

    if( ++i >= x ) return 0;
    strncpy( key, curr->l+i, x-i );
    key[x-i] = '\0';
    if( (s = find_alias( key ) ) == NULL ) return 0;
    x = i;
    subst_string( key, s );
    x = i + strlen( s );
    redraw_line(y, curr->l, line_num);
    return 1;
}

/**
*/
int		TED::define_key()
{
    int key, multi, *def;

    prompt("Choose the desired KEY !");
    key = get_pref_ch();
    prompt( "How many times should the command repeat ?" );
    multi = get_val(input());
    prompt("Enter command >");
    def = get_cmd_str();
    key_definition(key, multi, def);
	prompt("");
    return 0;
}

/**
*/
void	TED::key_definition(int key, int multi, int *def)
{
    int i;
    for( i = 0; i< n_kdefs; ++i ) if( key == key_defs[i].key ) break;

    if( i == n_kdefs ) {
	key_defs = (kdef*) (n_kdefs ?
	    realloc( key_defs, sizeof(kdef)*++n_kdefs) :
	    malloc( sizeof(kdef) * ++n_kdefs ) );
	i = n_kdefs-1;
	key_defs[i].key = key;
    }
    key_defs[i].multi = multi;
    memcpy( key_defs[i].meaning, def, TED_LSZ*sizeof(int) );
}

/**
*/
void	TED::manage_key(int c)
{
	int		i;

    for( i = 0; i < n_kdefs; ++i )	{
    	if ( c == key_defs[i].key )
    		break;
		}

    if ( i == n_kdefs )  {
		if ( c == TEDK_DELETE || c == TEDK_BSPACE || is_print( c ) )	{
		    if ( macro_changed )	{
				def_macro_ndx = 0;
				macro_changed = 0;
			    }
            if	( def_macro_ndx < TED_LSZ )
			    def_macro[def_macro_ndx++] = c;
			}
		else
			macro_changed = 1;

		primitive_key(c);
	    }
    else
    	rep_command( key_defs[i].multi, key_defs[i].meaning );
}

/**
*/
void	TED::save_defs(char *f_name)
{
    int i, j;
    FILE *fptr;
    char string[TED_LSZ];

    if( !f_name ) return;

    fptr = fopen( f_name, "w" );
    if( fptr == NULL ) {
		prompt( "Cannot write to file !" );
		return;
	    }

    fprintf( fptr, "# TED 2.0 - 1990 (C) Fernando Joaquim Ganhao Pereira !\n" );
    fprintf( fptr, "# Lisbon - Portugal .\n#\n#\n" );
    fprintf( fptr, "# Alias definitions:\n" );
    for( i = 0; i < n_alias; ++i )
       fprintf(fptr,"\"%s\"\t== \"%s\"\n",alias_list[i].key,alias_list[i].subs);
    fprintf( fptr, "\n" );
    fprintf( fptr, "# Key defenitions: (consult /usr/include/curses.h)\n" );
    fprintf( fptr, "# Key\tMult\tMeaning\n" );
    for( i = 0; i < n_kdefs; ++i ) {
	fprintf(fptr,"0%o\t%d\t :",key_defs[i].key,key_defs[i].multi);
	for( j = 0; j < TED_LSZ && key_defs[i].meaning[j] != 0; ++j )
	    fprintf( fptr, " 0x%02x", key_defs[i].meaning[j] );
	fprintf( fptr, "\n" );
    }
    fprintf( fptr, "\nPrefix = %03x HEX\n", pref_key );
    fprintf( fptr, "%s\n", indent ? "Indent" : "NoIndent" );
    fprintf( fptr, "%s\n", insert ? "Insert" : "Overwrite" );
    fprintf( fptr, "%s\n", sensit ? "Sensitive" : "Insensitive" );
    fclose( fptr );
    
    sprintf( string, "Definitions saved to file \"%s\".", f_name );
    prompt( string );
}

/**
*/
void	TED::read_defs(char *f_name)
{
    FILE *fptr;
    char b[TED_LSZ];
    char key[TED_MKEYS+1];
    char def[4*TED_MKEYS+1];
    int k, multi, meaning[TED_LSZ];
    int i, j;

    if( !f_name ) 
    	return;

    fptr = fopen( f_name, "r" );
    if( fptr == NULL ) {
		prompt( "Cannot read defs file!" );
		return;
	    }

    while ( fgets( b, TED_LSZ, fptr ) )	{
		if( strcmp( b, "\n" ) == 0 ) 
        	break;
		if( b[0] == '#' ) 
        	continue;
		if( sscanf( b, "%*c%s%*c %*2c %*c%[^\n]", key, def ) != 2 )
        	continue;
		key[strlen(key)-1] = '\0';
		def[strlen(def)-1] = '\0';
		create_alias( key, def );
	    }

    while ( fgets( b, TED_LSZ, fptr ) )	{
		if( strcmp( b, "\n" ) == 0 ) 
        	break;
		if( b[0] == '#' ) 
        	continue;
		if( sscanf( b, "%o %d", &k, &multi ) != 2 ) 
        	continue;
		for( i = 0; b[i] != ':' && i < strlen(b); ++i );
		j = 0;
		while( i < strlen( b ) ) {
		    while( b[i] && b[++i] != ' ' );
		    if( b[i] ) sscanf( b+i, " %*2c%x", &meaning[j++] );
			}
		meaning[j] = 0;
		if( j > 0 ) key_definition( k, multi, meaning );
    	}
        
    fgets( b, TED_LSZ, fptr );
    if( sscanf( b, "%*s %*c %x", &pref_key ) != 1 ) pref_key = TEDK_TEDK;	// PREF_KEY ???
    fgets( b, TED_LSZ, fptr );
    indent = !strncmp( b, "Indent", 6 );
    fgets( b, TED_LSZ, fptr );
    insert = !strncmp( b, "Insert", 6 );
    fgets( b, TED_LSZ, fptr );
    sensit = !strncmp( b, "Sensitive", 9 );
    fclose( fptr );
}

/**
*/
void	TED::print_defs()
{
    int i, j;

    clrscr();
    printw( "\nAlias definitions:\n" );
    for( i = 0; i < n_alias; ++i )
       printw( "\"%s\"\t== \"%s\"\n",alias_list[i].key,alias_list[i].subs);
    printw( "\n\n" );
    printw( "Key defenitions:\n" );
    printw( "Key\tMult\tMeaning\n" );
    for( i = 0; i < n_kdefs; ++i ) {
	if( is_print( key_defs[i].key ) )
	     printw( "\'%c\'\t%d\t : ",key_defs[i].key,key_defs[i].multi);
	else printw( "0%o\t%d\t : ",key_defs[i].key,key_defs[i].multi);
	for( j = 0; j < TED_LSZ && key_defs[i].meaning[j] != 0; ++j )
	    if( is_print( key_defs[i].meaning[j]) )
		 printw( "\'%c\',", key_defs[i].meaning[j] );
	    else printw( "0x%02x,", key_defs[i].meaning[j] );
	printw( "\n" );
    }
    printw( "Prefix = %03x HEX [code]\n", pref_key );
    printw( "\n%s\t", indent ? "Indent" : "NoIndent" );
    printw( "%s\n", insert ? "Insert" : "Overwrite" );
    printw( "Case Sensitivity %s\n", sensit ? "On" : "Off" );
    wait_any_key();
    redraw_screen();
}

/**
*/
void	TED::select_warea(int n)
{
    char m[TED_LSZ];

    if( n < 0 || n >= TED_MAX_WAREAS ) {
		prompt( "Invalid Buffer Number !" );
		return;
	    }

	current_warea = n;
    redraw_screen();
}

/**
*/
void	TED::read_to_warea(char *string)
{
    int bnumber;

    if( toupper(string[1]) == 'B' ) {
		bnumber = get_val( string+2 );
		read_buffer( bnumber, 1, wareas[bnumber]._buffer_size );
		redraw_screen();
	    }
    else if ( strlen( string ) > 2 ) {
		if( read_file( string+2, true ) ) 
			redraw_screen();
    	}
    else 
    	prompt( "Invalid READ command !" );
}

/**
*/
void	TED::read_buffer(int n, int li, int lf)
{
    line *ptr;
    int number;
    char string[TED_LSZ];

    if( n < 0 || n >= TED_MAX_WAREAS || n == current_warea ) {
		prompt( "Invalid Buffer Number !" );
		return;
	    }

    if ( li > lf ) { 
    	number = li; 
    	li = lf; 
    	lf = number; 
    	}

    ptr = &wareas[n]._head;
    number = 0;
    while( number < lf ) {
		ptr = ptr->next;
		if( ptr == &head ) break;
		++number;
		if( number >= li ) curr = insert_newline( curr, ptr->l );
	    }
    curr = NULL;
    goto_line( line_num );
    sprintf(string, "Read %d lines from buffer %d !", lf-li, n);
    prompt( string );
    changed = true;
}

/**
*/
void	TED::write_warea(char *string)
{
    int buff;
    int lim_inf = 1, lim_sup = buffer_size;
    char name[TED_LSZ];

    strcpy( name, fname );

    if( toupper( string[1] ) == 'B' ) buff = get_val( string+2 );
    else buff = -1;

    while( *string && !isspace( *string ) ) ++string;

    if( *string ) {
	if( *++string == '[' ) {
	    lim_inf = get_val( ++string );
	    while( *string && *string != ',' &&
		   *string != ';' && *string != ':' ) ++string;
	    lim_sup = get_val( ++string );
	    while( *string && *string != ']' ) ++string;
	    if( *string++ != ']' ) {
		prompt( "Invalid range !" );
		return;
	    }
	}
	while( *string && isspace( *string ) ) ++string;
	if( strlen( string ) ) strcpy( name, string );
    }
    if( buff >= 0 ) write_to_buffer( buff, lim_inf, lim_sup );
    else if( save_file( name, lim_inf, lim_sup, true) &&
	     strcmp( name, fname ) == 0 &&
	     lim_inf == 1 && lim_sup == buffer_size ) changed = false;
}

/**
*/
void	TED::write_to_buffer(int n, int  li, int lf)
{
    int i;
    line *ptr, *store_ptr, *new_line;

    if( n < 0 || n >= TED_MAX_WAREAS || n == current_warea ) {
	prompt( "Invalid Buffer Number !" );
	return;
    }
    if( li > lf ) { i = li; li = lf; lf = i; }

    store_ptr = wareas[n]._curr;
    ptr = &head;
    i = 0;
    do {
	ptr = ptr->next;
	++i;
	if( i < li || i > lf ) continue;
	new_line = (line *)malloc( sizeof( line ) );
	new_line->l = (char *)malloc( strlen( ptr->l ) + 1 );
	strcpy( new_line->l, ptr->l );
	store_ptr->next->prev = new_line;
	new_line->next = store_ptr->next;
	new_line->prev = store_ptr;
	store_ptr = store_ptr->next = new_line;
	++wareas[n]._buffer_size;
    } while( ptr != &head );

    wareas[n]._changed = true;
}

/**
*/
void	TED::delete_range(int li, int lf)
{
    int i;
    line *ptr;
    char string[TED_LSZ];

    if( li > lf ) { i = li; li = lf; lf = i; }

    ptr = &head;
    i = 0;
    while( i <= lf ) {
	ptr = ptr->next;
	++i;
	if( i > li ) delete_line( ptr->prev );
	if( ptr == &head ) break;
    }
    if( head.next == &head ) insert_newline( &head, "\n" );

    changed = true;
    screen_top = &head;
    curr = NULL;
    goto_line( li );
    sprintf( string, "%d lines deleted.", lf - li + 1 );
    prompt( string );
}

/**
*/
void	TED::erase_working_area(int n)
{
    line *ptr, *prev;

    if( n < 0 || n >= TED_MAX_WAREAS ) {
		prompt( "Invalid Buffer Number !" );
		return;
	    }

    ptr = wareas[n]._head.next;
    wareas[n]._head.prev = wareas[n]._head.next = &wareas[n]._head;
    wareas[n]._screen_top = wareas[n]._curr = &wareas[n]._head;
    while ( ptr != &wareas[n]._head )	{
    	prev = ptr;
		ptr = ptr->next;
		free(prev->l);
		free(prev);
	    }

    wareas[n]._x = wareas[n]._y = wareas[n]._act_x = 0;
    wareas[n]._line_num = wareas[n]._buffer_size = 1;
    wareas[n]._changed = true;
}

/**
*	set bookmark n to the current position
*/
void	TED::mark(int n)
{
    // remove previous bookmark
    if	( l_mark[n] )	{
		int		oy = con_y(l_mark[n]);

        if	( oy >= 0 && oy < screen_lines )	{
			gotoxy(screen_cols-1, oy);
            addch(' ');
            }
        }
    
	// setup bookmark
    x_mark[n] = x;
    l_mark[n] = line_num;

    // show
    prompt("bookmark %d, defined.", n);
	set_color(TED_BOOKMARK_COLOR);
	gotoxy(screen_cols-1, y);
	addch(n+'0');
	set_color(TED_TEXT_COLOR);
}

/**
*	set selection mark
*/
void	TED::selmark()
{
	if	( sel_style == 0 )
	    sx_mark = x;
	else
	    sx_mark = 0;

    sl_mark = line_num;
}

/**
*	Jumps to specified bookmark
*/
void	TED::recall_mark()
{
	int		n;
    char	*s;

	n = iask("Go to mark ?");
    if	( n >= 0 && n <= 9 )	{
       	if	( l_mark[n] == 0 )	
        	prompt("Bookmark undefined!");
		else	{
		    x = act_x = x_mark[n];
		    goto_line(l_mark[n]);
			prompt("Jump to %d line...", l_mark[n]);
			}
		}
	else
       	prompt("Invalid number!");
}

/**
*	user: delete selected text
*/                            
void	TED::user_delsel()
{
    int		l_start, l_end;
    int		x_start, x_end;
    int		cols, lines;
	undo_node_t	*unode;
    int		ucount = 0;

   	if	( sl_mark == 0 )
    	return;
    
	// selection - start
	l_start = sl_mark;
    x_start = sx_mark;

    // selection - end
    l_end   = line_num;
	if	( sel_style == 0 )
	    x_end = x;
	else
		x_end = EOL(curr);

    sl_mark = 0;	// reset selection

	undo_move();  ucount ++;
    if ( l_start > l_end )	{
    	// swap marks
        int		sw;

        sw = l_start; l_start = l_end; l_end = sw;
        sw = x_start; x_start = x_end; x_end = sw;
    	}
	else if ( l_start == l_end && x_start > x_end )	{
    	// swap x
        int		sw;
        
        sw = x_start; x_start = x_end; x_end = sw;
        }

	lines = l_end - l_start;
	cols  = x_end - x_start;
    ///////

    if	( lines == 0 )	{
    	//
    	//	inside a line
        //
    	char	*buf;
        
        if	( cols == 0 )	{
        	cols ++;
            x_end ++;
            }

		undo_add_line(current_warea, x_start, l_start, curr->l);
                    	
		buf = strdup(curr->l);
        buf[x_start] = '\0';
        strcat(buf, curr->l+x_end);
        if	( buf[strlen(buf)-1] != '\n' )
        	strcat(buf, "\n");
        strcpy(curr->l, buf);
        free(buf);

		unode = undo_move();
        unode->count = ucount + 1;
        x = x_start;
		refresh_current_line();
		}    
	else	{
    	//
    	//	block
        //
        char	*prev_clip;
        char	*buf;
        line	*ptr;

        ptr = get_line_node(l_start);
        curr = ptr;
        prev_clip = get_clip();
        sl_mark = l_start;	sx_mark = x_start;
        user_copy();
        sl_mark = 0;
        buf = get_clip();
        curr = ptr;
		undo_add_block(current_warea, x_start, l_start, buf);		// restore this text
        delete_block(x_start, l_start, strlen(buf));
        free(buf);
        set_clip(prev_clip);
        free(prev_clip);
        adjust_screen(true);
    	}
        
    prompt("block deleted");
}

/**
*	user: copy selected-text to clipboard
*/
void	TED::user_copy()
{
    int		l_start, l_end;
    int		x_start, x_end;
    int		cols, lines;
//  undo_node_t	*unode;

   	if	( sl_mark == 0 )
    	return;
    
    if ( current_warea == 0 ) {
    	beep(); 
        prompt("cannot copy/paste in clipboard!");
        return; 
        }

	// selection - start
	l_start = sl_mark;
    x_start = sx_mark;

    // selection - end
    l_end   = line_num;
	if	( sel_style == 0 )
	    x_end = x;
	else	{
		x_start = 0;
		x_end = 0;
		if	( l_end >= l_start )
			l_end ++;
		else
			l_start ++;
		}

    sl_mark = 0;	// reset selection

    if ( l_start > l_end )	{
    	// swap marks
        int		sw;

        sw = l_start; l_start = l_end; l_end = sw;
        sw = x_start; x_start = x_end; x_end = sw;
    	}
	else if ( l_start == l_end && x_start > x_end )	{
    	// swap x
        int		sw;
        
        sw = x_start; x_start = x_end; x_end = sw;
        }

	lines = l_end - l_start;
	cols  = (x_end - x_start) + 1;
    ///////
   	undo_clip();
//	unode = undo_move();
//  unode->count = 1;
    
    if	( lines == 0 )	{
    	//
    	//	inside a line
        //
        
    	char	*buf;
        
		buf = (char *) malloc(TED_LSZ);
		strcpy(buf, curr->l+x_start);
        if	( strlen(buf) > cols )
	        buf[cols] = '\0';
		strcat(buf, "\004\n");
        set_clip(buf);
        free(buf);
    	}
    else	{
    	//
    	//	block
        //
        
    	char	*buf = NULL;
        line	*ptr;
		int		i, size = 1;

        if	( x_end )
        	lines ++;
            
        ptr = get_line_node(l_start);
    	for ( i = 0; i < lines; i ++ )	{
        	char	*str;

        	// get the string
        	if	( i == 0 && x_start )
				// first line
            	str = strdup(ptr->l + x_start);
            else  if ( (i == (lines - 1)) && (x_end != 0) )	{
				// last line
				str = (char *) malloc(TED_LSZ);
				strcpy(str, ptr->l);
				str[x_end+1] = '\004';	// EOT, TED requires '\n' on each line, so '\x4' used for EOT mark
				str[x_end+2] = '\0';
                }
            else
            	str = strdup(ptr->l);

            // append the string
           	size += strlen(str)+1;
            if	( !buf )	{
            	buf = (char *) malloc(size);
                strcpy(buf, str);
                }
            else	{
            	buf = (char *) realloc(buf, size);
				strcat(buf, str);
                }

            // next
			free(str);
			ptr = ptr->next;
        	}

		if	( buf )	{
        	set_clip(buf);
	        free(buf); 
            }
		}            
    
    // notify
	clip_copy();
    prompt("block copied to the scrap");
}

/**
*	copy the current line
*/
void	TED::user_linecopy()
{
	undo_add_block(current_warea, x, y, curr->l);
    set_clip(curr->l);
	clip_copy();	// notify app
    prompt("line copied to scrap.");
}

/**
*	cut the current line
*/
void	TED::user_linecut(bool storeit)
{
	undo_add_block(current_warea, x, line_num, curr->l);		// restore this line
    if	( storeit )	{
		undo_node_t	*unode;
	    unode = undo_clip();										// restore clipboard
	    unode->count = 1;											// execute 2 undo nodes
    
	    set_clip(curr->l);
	    clip_copy();			// notify
        }

    if ( curr->next == &head && curr->prev == &head )	{
        free(curr->l);
        curr->l = strdup("\n");
        }
    else if ( curr->next == &head )	{
//    	curr = curr->prev;
//	    delete_line(curr->next);
		strcpy(curr->l, "\n");
        }
	else {
    	curr = curr->next;
	    delete_line(curr->prev);
    	}

	if	( storeit )    
	    prompt("line deleted to scrap.");
	// else line deleted        
    redraw_screen();					// redraw screen
}

/**
*	paste clipboard's data
*/
void	TED::paste_buffer()
{
	char	*buf, *p;
    int		lines;

	clip_paste();						// notify upper class
    buf = get_clip();					// get clipboard's data
	p = strrchr(buf, '\004');
	if	( p )	*p = '\0';
	lines = count_lines(buf);			// count text lines
    if	( lines )	{
		undo_add_rmblock(current_warea, x, line_num, buf);	// undo info: remove block
	    set_text(buf, current_warea, curr);	// insert them
		}
	else	{
		char	*right_part;
    	undo_line();        			// undo info: one line to be fixed

		right_part = strdup(curr->l+x);	// rightest part of string (at least the '\n')
		curr->l = (char *) realloc(curr->l, strlen(buf) + strlen(curr->l) + 1);
		strcpy(curr->l+x, buf);
		strcat(curr->l, right_part);
		free(right_part);
		}
    free(buf);							// release buffer
    changed = true;						// set the 'modified' flag
    goto_line(line_num - lines);		// go to correct line
	adjust_screen(true);				// redraw screen
}

/**
*	return the buffer 0 (clipboard) as newly allocated string (malloc)
*/
char	*TED::get_clip()
{
	return get_text(0);
}

/**
*	sets clipboard's text
*/
void	TED::set_clip(const char *text)
{
    if ( current_warea == 0 ) {
    	beep(); 
        prompt("cannot copy/paste in clipboard!");
        return; 
        }

	set_text(text, 0);
	wareas[0]._changed = true;
}

/**
*	match parenthesis and other pairs
*/
void	TED::match()
{
    int		c = 1;
	char	fnd = curr->l[x];
    line* 	ptr = curr;
    int 	number = line_num;
    char 	obj;
    int 	i = x;

    switch( curr->l[x] ) {
	case '{' : obj = '}'; break;
	case '[' : obj = ']'; break;
	case '(' : obj = ')'; break;
	case '}' : obj = '{'; break;
	case ']' : obj = '['; break;
	case ')' : obj = '('; break;
	default  : 
    	return;
	    }

    if( fnd == '}' || fnd == ']' || fnd == ')' ) {
	while( c > 0 ) {
	    if( --i < 0 ) {
		ptr = ptr->prev;
		if( --number <= 0 ) {
		    beep();
		    return;
		}
		i = EOL( ptr );
	    }
	    if( ptr->l[i] == fnd ) ++c;
	    else if( ptr->l[i] == obj ) --c;
	}
    }
    else {
	while( c > 0 ) {
	    if( ++i > EOL(ptr) ) {
		ptr = ptr->next;
		if( ++number > buffer_size ) {
		    beep();
		    return;
		}
		i = 0;
	    }
	    if( ptr->l[i] == fnd ) ++c;
	    else if( ptr->l[i] == obj ) --c;
	}
	
    }
    x = act_x = i;
    goto_line( number );
}

/**
*	buffer list
*/
void	TED::user_buflist()
{
	char	*list, *buf;
	int		i, area, len, lsz;

	// init
	buf = (char *) malloc(2048);
	list = NULL;

	// build the table
	for ( i = 0; i < TED_MAX_WAREAS; i ++ )	{
		sprintf(buf, "%c %-16s%5d %s\n",
			wareas[i]._changed  ? '*' : ' ',
			basename(wareas[i]._fname), 
			wareas[i]._buffer_size, 
			wareas[i]._fname);

		len = strlen(buf);
		if	( list )	{
			lsz += len;
			list = (char *) realloc(list, lsz+2);
			strcat(list, buf);
			}
		else	{
			lsz = len;
			list = (char *) malloc(lsz+2);
			strcpy(list, buf);
			}
		}

	//
	area = select_from_list("buffer list", list, TED_MAX_WAREAS, current_warea);
	if	( area != -1 )	{
       	select_warea(area);
	    prompt("'%s': buffer activated", fname);
		}
	redraw_screen();

	// clean up
	free(buf);
	free(list);
}

/**
*/
char	*TED::colfiles(char *dir, char *wc)
{
    char name[1024];
	struct dirent	*dp;
    DIR 	*dfd;
	struct	stat	st;
	char	*result;
	int		len, size;
	
	if ((dfd = opendir(dir)) == NULL) 
        return NULL;

	result = (char *) malloc((size = 0x10000));
	*result = '\0';

    while ((dp = readdir(dfd)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue; /* skip self and parent */
        if ( strlen(dir)+strlen(dp->d_name)+2 > sizeof(name) )
			;
        else {
			sprintf(name, "%s/%s\n", dir, dp->d_name);
			len = strlen(name);
			if	( len+strlen(result)+2 >= size )	{
				size += 0x10000;
				result = (char *) realloc(result, size);
				}
			strcat(result, name);
			
			/* check filename */
//			if	( !wc )
//				callusr = 1;
//			else	
//				callusr = wc_match(wc, dp->d_name);

//			stat(name, &st);
//			if	( st.st_mode & S_IFDIR )	
//				dirwalk(name, wc, use_ip);
        	}
	    }
    closedir(dfd);

	return result;
}

/**
*	file list
*/
void	TED::filelist(const char *prefix)
{
	char	*list;
	int		fnum;
	char	cwd[1024];

	// init
	getcwd(cwd, 1024);
	list = colfiles(cwd, "*");

	//
	fnum = select_from_list("file list", list, -1, 0);
	if	( fnum != -1 )	{
		// file = list[fnum]
		free(list);
		// if file = dir, redo
		// else load file
		;
		}
	else
		free(list);

	redraw_screen();
}

/**
*	interface: select_from_list
*
*	@param title the window title
*	@param list the list (a string-items separated by '\n')
*	@param count the number of elements in list (-1 for auto)
*	@param defsel the default selection
*/
int		TED::select_from_list(const char *title, const char *list, int count, int defsel)
{
    int		sel, c, fs = 0, last_fs = 1, i, max_l, len;
	char_p	*tstr;
	const char *ps, *p;
	bool	bexit = false;

	// no count specified
	if ( count == -1 )	{
		count = 0;
		p = list;
		while ( *p )	{
			if	( *p == '\n' )	
				count ++;
			p ++;
			}
		}

	// no items
	if	( count == 0 )
		return -1;

	// build string table
	tstr = (char_p *) malloc(sizeof(char_p) * count);
	ps = p = list;
	for ( i = 0; i < count; i ++ )	{
		while ( *p )	{
			if	( *p == '\n' )	{
				len = strchr(ps, '\n') - ps;
				tstr[i] = (char *) malloc(len+1);
				memcpy(tstr[i], ps, len);
				tstr[i][len] = '\0';
				p ++;
				ps = p;
				break;
				}
			p ++;
			}
		}

	// init screen
    max_l = screen_lines - 2;
	sel   = defsel;
	in_editor_scr = false;

    while ( true ) {

       	// see if must scroll
    	if	( last_fs + sel > max_l )
	       	fs = sel - max_l;
        else
        	fs = 0;
    
       	// redraw the list
		if	( fs != last_fs )	{    
		    clrscr();
            header(0, title);
		    for ( i = fs; i < count; i ++ )	{
				select_from_list_draw_node(fs, i, tstr[i], false);
	        	if	( (i-fs) == max_l )
		        	break;
		        }

            last_fs = fs;
		    prompt("use <tab>, <space> or the cursor keys to select a buffer; <esc> or <q> to return back");
			}            

        // current
		select_from_list_draw_node(fs, sel, tstr[sel], true);
	    gotoxy(0, (sel-fs)+1);
		refresh();
		c = get_pref_ch();
		select_from_list_draw_node(fs, sel, tstr[sel], false);
        
        //
		switch ( c ) {
		case 'q':	case 'Q':
	    case TEDK_ESCAPE:
			sel = -1;
			bexit = true;
			break;
	    case TEDK_HOME:
	    case TEDK_PREVSCR:
        	sel = 0;
			break;
	    case TEDK_EOL:
	    case TEDK_NEXTSCR:
        	sel = 0;
			break;
	    case TEDK_TAB:
	    case ' ' :
	    case TEDK_DOWN:
	    case TEDK_RIGHT:
			if ( ++ sel >= count ) 
         		sel = 0;
			break;
	    case TEDK_UP:
	    case TEDK_LEFT:
	    case TEDK_DELETE:
	    case TEDK_BSPACE:
        	if ( -- sel < 0 ) 
            	sel = count - 1;
			break;
	    case TEDK_RETURN:
			bexit = true;
			break;
	    default:
        	beep();
			}

		//
		if	( bexit )
			break;
    	}

	// free string table
	for ( i = 0; i < count; i ++ )	
		free(tstr[i]);
	free(tstr);

	//
	in_editor_scr = true;
   	redraw_screen(); 
	return sel;
}

/**
*	print info about the buffer i (area)
*/
void	TED::select_from_list_draw_node(int fs, int i, const char *src, bool sel)
{
	int		len;
	char	*buf;

	buf = (char *) malloc(strlen(src)+16);
    gotoxy(0, (i-fs)+1);
	
	if	( sel )	{
		set_color(TED_SELCOLOR);
		strcpy(buf, ">>> ");
		}
	else	{
		set_color(TED_COLOR);
		strcpy(buf, "    ");
		}
	strcat(buf, src);

	// clipping
	len = strlen(buf);
	if	( len >= screen_cols )	{
		buf[screen_cols] = '\0';
		set_color(TED_XSCROLL_COLOR);
		buf[screen_cols-1] = '>';
		}
	printw(buf);

	//
    clreol();
	set_color(TED_TEXT_COLOR);
	free(buf);
}

//
int		TED::input_remove_char(char *dest, int pos)
{
	char	cstr[3];
	int		count, remain;
	char	*buf;

	if	( dest[pos] )	{
		count = 1;

		remain = strlen(dest+pos+1);
		buf = (char *) malloc(remain+1);
		strcpy(buf, dest+pos+count);

		dest[pos] = '\0';
		strcat(dest, buf);
		free(buf);
		return count;
		}
	return 0;
}

/**
*	get a string from console, returns a static string, str[0] = 0 for escape
*/
char	*TED::input(const char *defval)
{
	char		*dest;
	int			c = 0;
	int			pos = 0, len = 0;
	int			prev_x = 1, prev_y = 1;
	int			w, replace_mode = 0;
	char		prev_ch;
	int			code, size = TED_LSZ-1;
    bool		first_key = true;

	dest = static_getstr;
	in_editor_scr = false;

    if	( defval )
    	strcpy(dest, defval);
    else
		*dest = '\0';

	getxy(&prev_x, &prev_y);

//	prev_x = strlen(last_message) + 1;
//	prev_y = scr_rows();
//	also, last_message can be used (avoid getxy)

	set_color(TED_STATUS_COLOR);

    gotoxy(prev_x, prev_y);
    clreol();
    gotoxy(prev_x, prev_y);
	printw("%s", dest);
	gotoxy(prev_x, prev_y);
    fflush(stdout);
    
	while ( c != TEDK_ENTER && c != TEDK_ESCAPE )	{
    	if	( (c = get_pref_ch()) != 0 )	{
			len = strlen(dest);

            if	( first_key )	{
            	if	( c == 9 || (c >= 32 && c < 127) || (c > 127 && c < 256) )	{
                	// erase the buffer
					dest[0] = '\0';
                    len = 0;
                	}
            	first_key = false;
            	}

			switch ( c )	{
			case TEDK_ENTER:
				dest[pos] = '\0';
				break;
			case TEDK_ESCAPE:
				dest[0] = '\0';
				break;
			case TEDK_HOME:
				pos = 0;
				break;
			case TEDK_EOL:
				pos = len;
				break;
			case TEDK_BSPACE: 		// backspace
				if	( pos > 0 )		{
					pos -= input_remove_char(dest, pos-1);
					len = strlen(dest);
					}
				else
					beep();
				break;
			case TEDK_DELETE: 		// delete
				if	( pos < len )		{
					input_remove_char(dest, pos);
					len = strlen(dest);
					}
				else
					beep();
				break;
			case TEDK_INSOVR:
				replace_mode = !replace_mode;
				break;
			case TEDK_LEFT:
				if	( pos > 0 )
					pos --;
				else
					beep();
				break;
			case TEDK_RIGHT:
				if	( pos < len )
					pos ++;
				else
					beep();
				break;
			default:
				if	( c < 256 )	{			// Not an hardware key
                	int		count = 1;
					char	cstr[3], *buf;
                    int		remain;
                    
                    cstr[0] = c;
                    cstr[1] = '\0';
                    
					if	( replace_mode )	{
                        // overwrite mode
                        remain = strlen(dest+pos);
                        buf = (char *) malloc(remain+1);
                        strcpy((char *) buf, (char *) dest+pos);
                        memcpy(dest+pos, cstr, count);
                        dest[pos+count] = '\0';

                        count = 1;

                        if	( buf[0] )	// not a '\0'
                            strcat((char *) dest, (char *) buf+count);
                        free(buf);
                        }
                    else	{
                        // insert mode
                        remain = strlen(dest+pos);
                        buf = (char *) malloc(remain+1);
                        strcpy((char *)buf, (char *) dest+pos);
                        memcpy( (char *)dest+pos, (char *)cstr, count);
                        dest[pos+count] = '\0';
                        strcat( (char *) dest, (char *) buf);
                        free(buf);
                        }
					pos ++;
                    }
				else
					c = 0;

				// check the size
				len = strlen(dest);
				if	( len >= (size-1) )
					break;
				}

			// draw
			gotoxy(prev_x, prev_y);
			clreol();
			printw("%s", dest);
			gotoxy(prev_x+pos, prev_y);
		    fflush(stdout);
			}	// inkey() loop

    	}
        
	set_color(TED_TEXT_COLOR);
    refresh();
    status_line("");
	in_editor_scr = true;
    
	return dest;
}


/**
*/
int		TED::wait_any_key()
{
    prompt( "Press any key to continue !" );
    return get_pref_ch();
}

/**
*/
void	TED::change_case()
{
    if( isupper( curr->l[x] ) ) curr->l[x] = tolower( curr->l[x] );
    else if( islower( curr->l[x] ) ) curr->l[x] = toupper( curr->l[x] );
    redraw_line( y, curr->l, line_num);
    changed = true;
    move_right();
}

/**
*/
void	TED::repeat_again()
{
    int i;
    
    for ( i = 0; i < def_macro_ndx; i ++ ) 
    	primitive_key(def_macro[i]);
}

/**
*	returns the whole file as a newly allocated string.
*	use free() to free the string
*/
char	*TED::get_text(int warea, line *start, int lines)
{
    line 	*ptr;
    int 	i, len = 0, count;
    char	*s;
	int		old_warea = current_warea;

    current_warea = warea;

    // starting from...
	if	( start )	
    	ptr = curr;
    else
	    ptr = head.next;

    count = 0;
    while ( ptr != &head )	{
        len += strlen(ptr->l);
		ptr = ptr->next;
        
        count ++;
        if	( lines == count )
        	break;
        }
        
	len ++;
    s = (char *) malloc(len+1);
	*s = '\0';    

    count = 0;
    ptr = head.next;
    while ( ptr != &head )	{
        len += strlen(ptr->l);
		strcat(s, ptr->l);
		ptr = ptr->next;
        
        count ++;
        if	( lines == count )
        	break;
		}

    current_warea = old_warea;
	return s;	
}

/**
*	sets or inserts a multiline text
*
*	str is the text to be inserted.
*	start is where to insert. if start is NULL the current working area 
*	will be replaced by the new text
*/
void	TED::set_text(const char *str, int warea, line *start)
{
    line *new_line, *ptr, *prev;
	const char	*p;
    char	*d;
    char	tex[TED_LSZ];
	int		old_warea = current_warea;

	current_warea = warea;

    if	( !start )	{
    	// change the whole buffer
        // so, first clear it
	    ptr = head.next;
	    head.next = head.prev = &head;
	    line_num = 0;
	    buffer_size = 0;
        
	    while ( ptr != &head )	{
	        prev = ptr;
	        ptr  = ptr->next;
	        free(prev->l);
	        free(prev);
	        }

	    //
	    ptr = curr = screen_top = &head;
        }
	else	{
    	// insert from that position, start cannot be the &head
    	ptr = start->prev;	// ptr = start->prev, because the first line will be inserted next to ptr
        }

    // start
    p = str;
    d = tex;
    while ( *p )	{
    	if	( *p == '\r' )	// windows 
        	;
        else if ( *p == '\n' )	{	// change line
        	*d = *p; d ++; *d = '\0';
            d = tex;
			ptr = insert_newline(ptr, tex);
            if	( start )
            	line_num ++;
        	}
        else
        	( *d = *p, d ++ );
		p ++;    
    	}

    // last line
	*d = '\0';
    if	( strlen(tex) )	{
    	strcat(tex, "\n");	// @#$!@$#@$!
		ptr = insert_newline(ptr, tex);
		if	( start )
           	line_num ++;
        }

	// finaly     
    if	( !start )	{
	    curr = screen_top = head.next;
	    x = act_x = 0;
        line_num = 1;
        }
	changed = true;

	// restore    
	current_warea = old_warea;
}

/**
*	load a file
*/
void	TED::load(const char *file, int warea)
{
	int		old_warea = current_warea;

    current_warea = warea;
        
	read_file(file, true, true);
   	strcpy(fname, file);
    goto_line(1); x = act_x = 0;
	changed = false;

    free_undo_table(warea);
    wareas[warea].undo_head  = wareas[warea].undo_tail = 0;
	wareas[warea].undo_table = (undo_node_t*) malloc(sizeof(undo_node_t) * TED_UNDO_NODES);
	memset(wareas[warea].undo_table, 0, sizeof(undo_node_t) * TED_UNDO_NODES);
	scan_stx(warea);
        
 	if	( old_warea != warea ) 
	    current_warea = old_warea;
	else	{
	    adjust_screen(true);
        place_cursor();
        }
}

/**
*	resets the buffer and sets the filename and the default text
*/
void	TED::newfile(const char *file, const char *deftext, int warea)
{
	int		old_warea = current_warea;

    current_warea = warea;
        
   	strcpy(fname, file);
    if	( deftext )
	    set_text(deftext, warea);
	else        
    	set_text("\n", warea);
    goto_line(1); x = act_x = 0;
	changed = false;

    free_undo_table(warea);
    wareas[warea].undo_head  = wareas[warea].undo_tail = 0;
	wareas[warea].undo_table = (undo_node_t*) malloc(sizeof(undo_node_t) * TED_UNDO_NODES);
	memset(wareas[warea].undo_table, 0, sizeof(undo_node_t) * TED_UNDO_NODES);
        
 	if	( old_warea != warea ) 
	    current_warea = old_warea;
	else	{
	    adjust_screen(true);
        place_cursor();
        }
}

/**
*	save a file 
*/
void	TED::save(const char *file, int warea)
{
	int		old_warea = current_warea;

	if	( old_warea != warea )    
	    select_warea(warea);

	if	( file == NULL )        
		save_file(fname, 1, buffer_size, true);
	else        
		save_file(file, 1, buffer_size, true);
    wareas[warea]._changed = false;	
	scan_stx(warea);

	if	( old_warea != warea )    
	    select_warea(old_warea);
}

/**
*	save a file 
*/
void	TED::save_to(const char *file, int warea)
{
	int		old_warea = current_warea;
	char	buf[1024];

	if	( old_warea != warea )    
	    select_warea(warea);

	strcpy(buf, fname);
	save_file(file, 1, buffer_size, false);
	strcpy(fname, buf);
	scan_stx(warea);

	if	( old_warea != warea )    
	    select_warea(old_warea);
}

/**
*	prints something to the status bar and keeps the last message (for redraw)
*
*	use prompt() instead of this
*/
void	TED::disp_status(const char *str)
{
	int		col;
    char	buf[TED_LSZ];

	col = real_col(x, curr->l);
	if	( str[0] )	{
		sprintf(buf, "%02d:%05d:%04d | %s", current_warea, line_num, col+1, str);
		strcpy(last_usermsg, str);
        }
    else	
		sprintf(buf, "%02d:%05d:%04d | %s", current_warea, line_num, col+1, last_usermsg);

    strcpy(last_message, buf);
	set_color(TED_STATUS_COLOR);
    status_line(buf);
	set_color(TED_TEXT_COLOR);
}

/**
*	sets the status line text
*/
void	TED::prompt(const char *fmt, ...)
{
	va_list argptr;
    char	buffer[TED_LSZ];

	va_start(argptr, fmt);
	vsprintf(buffer, fmt, argptr);
	va_end(argptr);
    disp_status(buffer);
}

/**
*	draw the status line
*/
void	TED::status_line(const char *say)
{
	gotoxy(0, scr_rows());
    clreol();
    if ( say )	{
		int		len = scr_cols();
		char	*buf;

		buf = (char *) malloc(len+1);
		snprintf(buf, len, " %s ", say);
		buf[len] = '\0';
		printw("%s", buf);
		free(buf);
		}
	else
		printw("");
}

/**
*	prints a prompt and gets a user string
*
*	@return the string or -1 for escape
*/
char*	TED::ask(const char *prompt, char *buf, int size, const char *defval)
{
	char	*s;

	set_color(TED_STATUS_COLOR);
	status_line(prompt);
	s = input(defval);
    strncpy(buf, s, size);
    buf[size-1] = '\0';
	set_color(TED_TEXT_COLOR);
    return buf;
}

/**
*	prints a prompt and gets a user number
*
*	@return the number or -1 for escape
*/
int		TED::iask(const char *prompt, int default_value)
{
	char	buf[64];
    char	sint[32];
	int		n;

    sprintf(sint, "%d", default_value);
    ask(prompt, buf, 64, sint);
    if	( buf[0] )
	    return atoi(buf);
 	return -1;
}

/**
*	some debug console for personal use :)
*/
void	TED::ndc_console()
{
    char	text[128], cmd[128];

    strcpy(text, "ndc$");
	while ( true )	{
//		ask(text, cmd, 128);
		prompt(text);
        strcpy(cmd, input());
        if	( strcmp(cmd, "q") == 0 )	{
			prompt("");
        	return;
            }
        if	( strcmp(cmd, "h") == 0 )	
        	sprintf(text, "qsch $");
		else if	( strcmp(cmd, "s") == 0 )	
        	sprintf(text, "%dx%d $", screen_cols, screen_lines);
		else if	( strcmp(cmd, "c") == 0 )	{
        	line	*ptr = &head;
            int		count = 0;

            ptr = ptr->next;
            clrscr();
            gotoxy(0,0);
            while ( ptr != &head )	{
            	if	( !ptr->l )	
                	printw("%d: null l\n", count);
	            ptr = ptr->next;
                count ++;
            	}
			printw("--- bufsize %d, count %d\n", buffer_size, count);                
        	}
        }
}

/**
*	returns true if the area has been modified
*/
bool	TED::modified(int warea)
{
	return (wareas[warea]._changed) ? true : false;
}

/**
*	print right-adjusted
*/
void	TED::lprintf(int sy, const char *fmt, ...)
{
	va_list	ap;
	char	msg[TED_LSZ];

	va_start(ap, fmt);
	vsnprintf(msg, TED_LSZ, fmt, ap);
	va_end(ap);

	gotoxy(0, sy);
	printw("<<< ");
    printw(msg);
	printw(" >>>");
}

/**
*	print right-adjusted
*/
void	TED::rprintf(int sy, const char *fmt, ...)
{
	va_list	ap;
	char	msg[TED_LSZ];
    int		i, len;

	va_start(ap, fmt);
	vsnprintf(msg, TED_LSZ, fmt, ap);
	va_end(ap);

	len = strlen(msg) + 2;
	gotoxy((screen_cols - len), sy);
	printw(" ");
    printw(msg);
	printw(" ");
}

/**
*	print center-adjusted
*/
void	TED::cprintf(int sy, const char *fmt, ...)
{
	va_list	ap;
	char	msg[TED_LSZ];
    int		i, cx, len;
	const char	*left = " ";
	const char	*right = " ";

	va_start(ap, fmt);
	vsnprintf(msg, TED_LSZ, fmt, ap);
	va_end(ap);

	len = strlen(msg) + strlen(left) + strlen(right);
	cx = (screen_cols/2 - len/2);
	if	( cx < 0 )
		cx = 0;
		
	gotoxy(cx, sy);
	printw(left);
    printw(msg);
	printw(right);
}

/**
*	footnote
*/
void	TED::footer(int sy, const char *fmt, ...)
{
	va_list	ap;
	char	msg[TED_LSZ];
	int		i;

	va_start(ap, fmt);
	vsnprintf(msg, TED_LSZ, fmt, ap);
	va_end(ap);

    gotoxy(0, sy);
	#if defined(_UnixOS)
	printw("\033[1m\033[30m");
	#endif
    set_color(TED_FOOTER_COLOR);
	for ( i = 0; i < screen_cols; i ++ )
//    	addch('_');
    	addch('~');

	cprintf(sy, msg);
	set_color(TED_TEXT_COLOR);
}

/**
*	output header
*/
void	TED::header(int sy, const char *fmt, ...)
{
	va_list	ap;
	char	msg[TED_LSZ];
	int		i;

	va_start(ap, fmt);
	vsnprintf(msg, TED_LSZ, fmt, ap);
	va_end(ap);

    footer(sy, msg);
}

/**
*	activate working-area
*/
void	TED::activate(int warea)
{
	select_warea(warea);
	redraw_screen();
}

/**
*/
bool	TED::modified(bool newval, int warea)
{
	return (wareas[warea]._changed = newval);
}

/**
*/
char	*TED::dlq_cp(char *dest, int maxcp, const char *delims, char *p)
{
	char	*d;
	int		count = 0;

	d = dest;
	while ( *p )	{	
		if	( strchr(delims, *p) )	{
			*p = '\0';
			break;
			}
		else
			*d ++ = *p;

		p ++;
		count ++;
		if	( count >= maxcp )
			break;
		}

	*d = '\0';
	p ++;
	return p;
}

/**
*	add a new syntax-table (syntax highlight)
*/
bool	TED::add_stx(const char *file, const char *ext, const char *sign)
{
	FILE	*fp;

	fp = fopen(file, "r");
	if	( fp )	{
		stx_t	*node;
		int		size;
		char	*ps, *p, *d;
		int		i;

		// create a new node
		if	( stx_table )	
			stx_table = (stx_t *) realloc(stx_table, sizeof(stx_t) * (stx_count+1));
		else
			stx_table = (stx_t *) malloc(sizeof(stx_t) * (stx_count+1));

		node = &stx_table[stx_count];
		stx_count ++;

		// set node's data
		memset(node, 0, sizeof(stx_t));
		if	( ext )			node->ext = strdup(ext);
		if	( sign )		node->sign = strdup(sign);

		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		node->csv = (char *) malloc(size+1);
		fread(node->csv, size, 1, fp);
		node->csv[size] = '\0';
		fclose(fp);

		// analyze data
		ps = p = node->csv;
		while ( *p )	{
			if	( *p == '\n' )	{
				*p = '\0';
				break;
				}
			p ++;
			}
		strcpy(node->desc, ps);
		p ++;

		p = dlq_cp(node->rem_block_b, 7, ":\n", p);		// comments block start
		p = dlq_cp(node->rem_block_e, 7, ":\n", p);		// comments block ends
		p = dlq_cp(node->rem_sym1, 7, ":\n", p);		// line-remarks 1
		p = dlq_cp(node->rem_sym2, 7, ":\n", p);		// line-remarks 2
		p = dlq_cp(node->rem_sym3, 7, ":\n", p);		// line-remarks 3

		// string character
		node->strq_ch = *p;
		while ( *p )	{ if ( *p == '\n' ) break; p ++; }
		if	( *p )	p ++;
	
		// convert \n to \0
		ps = p;
		node->count = 0;
		while ( *p )	{
			if	( *p == '\n' )	{
				*p = '\0';
				node->count ++;
				}

			p ++;
			}

		// build index
		node->index = (int *) malloc(node->count * sizeof(int));
		p = ps;
		for ( i = 0; i < node->count; i ++ )	{
			node->index[i] = (int) p;
			while ( *p ) 	p ++; 
			p ++;
			}

		return true;
		}

	return false;
}

/**
*	sets syntax-highlight code 
*/
void	TED::scan_stx(int warea)
{
	int		i;
	const char	*ext;

	ext = strrchr(wareas[warea]._fname, '.');
	for ( i = 0; i < stx_count; i ++ )	{
		if	( ext )	{
			if	( stx_table[i].ext )	{
				if	( strcmp(stx_table[i].ext, ext) == 0 )	{
					wareas[warea]._hsyntax = i + 1;
					break;
					}
				}

//			if	( stx_table[i].sign )	{
//				check the first bytes of the file
//				}
			}
		}
}

/**
*	sets syntax-highlight code 
*/
void	TED::set_stx(int warea, int code)
{
	wareas[warea]._hsyntax = code + 1;
}

/**
*	search keywords (syntax)
*/
bool	TED::stx_search(stx_t *node, const char *source, bool caseless)
{
	int		i;
	char	*key;

	for ( i = 0; i < node->count; i ++ )	{
		if	( caseless )	{
			if	( strcasecmp(source, (char *) node->index[i]) == 0 )
				return true;
			}
		else	{
			if	( strcmp(source, (char *) node->index[i]) == 0 )
				return true;
			}
		}
	return false;
}

/**
*	word help
*/
void	TED::user_help_word()
{
	int		xend = 0;
	int		xstart;
	int		xlen;
	char	*buf;

	// find the begin of the word
    if ( x == 0 )	
		xstart = x;	
	else	{
		xstart = x;
		while ( xstart )	{
			if	( !(isalnum(curr->l[xend]) || curr->l[xend] == '$' || curr->l[xend] == '_') )
				break;
			xstart --;
			}

		if	( !(isalnum(curr->l[xend]) || curr->l[xend] == '$' || curr->l[xend] == '_') )
			xstart ++;

		if	( !(isalnum(curr->l[xend]) || curr->l[xend] == '$' || curr->l[xend] == '_') )
			return;	// not found
		}

	// find the end of the word
	xend = xstart;
	while ( isalnum(curr->l[xend]) || curr->l[xend] == '$' || curr->l[xend] == '_' )	
		xend ++;

	xlen = xend - xstart;
	if	( xlen )	{
		buf = (char *) malloc(xlen+1);
		strncpy(buf, curr->l+xstart, xlen);
		buf[xlen] = '\0';
		// call upper class to show help
		help_on_word(buf);
		free(buf);
		}
}



