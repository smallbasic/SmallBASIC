/**
*/

#include <vcl.h>
#include <stdarg.h>
#include <stdio.h>
#include "TedBCB.h"
#include <clipbrd.hpp>

TedBCB::TedBCB(TPaintBox *imgControl) : TED()
{
	img = imgControl;
	win = (TWinControl *) img;
//    canvas = asgn_canvas;
	bmp = NULL;
    strcpy(font_name, "Courier New");
    font_size = 9;
    lock_refresh = false;
	paint();

    fx = canvas->TextWidth("Q");
    fy = canvas->TextHeight("Q");
    cx = cy = 0;
    k_head = k_tail = 0;
    store_keys = false;
    Enabled = true;
}

TedBCB::~TedBCB()
{
	delete bmp;
}

// clear screen
void	TedBCB::clrscr()
{
	TRect	r;

    r = canvas->ClipRect;
	canvas->FillRect(r);

    fx = canvas->TextWidth("Q");
    fy = canvas->TextHeight("Q");
    cx = cy = 0;
}

// clear to EOL
void	TedBCB::clreol()
{
	TRect	r;
    
    r = canvas->ClipRect;
    r.top += cy * fy;
    r.bottom = r.top + fy;
    r.left = cx * fx;
	canvas->FillRect(r);
}

// set cursor position
void	TedBCB::gotoxy(int x, int y)
{
	cx = x; // - 1;
    cy = y; // - 1;
    if	( cy > scr_rows() )
    	cy = scr_rows();
}

// delete the line at cursor and scroll up the rests
void	TedBCB::deleteln()
{
	win->Invalidate();
}

// insert an empty line at cursor and scroll down the rests
void	TedBCB::insertln()
{
	win->Invalidate();
}

// printf    
void	TedBCB::printw(const char *fmt, ...)
{
	va_list argptr;
    char	buf[TED_LSZ];
    char	*p, *ps;
    int		ts = get_tab_size();

	va_start(argptr, fmt);
	vsprintf(buf, fmt, argptr);
	va_end(argptr);

   	ps = p = buf;
    while ( *p )	{
      	if	( *p == '\n' )	{
           	*p = '\0';
             if	( strlen(ps) )	
                canvas->TextOut(cx * fx, cy * fy, ps);
                    
            cx = 0;
            cy ++;
		    if	( cy > scr_rows() )	{
            	// scroll up
		    	cy = scr_rows();
                insertln();
                }
                
            ps = p+1;
            }
        else if	( *p == '\t' )	{
            int		len;
            char	*str;
            
            *p = '\0';
            if	( strlen(ps) )	
                canvas->TextOut(cx * fx, cy * fy, ps);
                    
            cx  += strlen(ps);
            len  = (ts - (cx % ts));
            str  = (char *) malloc(len+1);
            memset(str, ' ', len);
            str[len] = '\0';
            canvas->TextOut(cx * fx, cy * fy, str);
            free(str);

            // next
            cx += len;
            ps = p+1;
            }

        p ++;
        }

    if	( strlen(ps) )	{
        canvas->TextOut(cx * fx, cy * fy, ps);
        cx += strlen(ps);
        }
}

// gets a key (similar to getch() but for extended keys uses > 0x100)
int		TedBCB::get_pref_ch()
{
	int		c;

    store_keys = true;
	while ( (c = inkey()) == 0 )	
    	Application->ProcessMessages();
    store_keys = false;
 	return c;
}

//
int		TedBCB::dev_input_remove_char(byte *dest, int pos)
{
	byte	cstr[3];
	int		count, remain;
	byte	*buf;

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


// get a string from console, returns a static string, str[0] = 0 for escape
char	*TedBCB::get_string(const char *defval)
{
	static char dest[TED_LSZ];
//
//	AnsiString s = InputBox("Input Box", "Prompt", "Default string");
//    strcpy(buf, s.c_str());

	int			c = 0;
	int			pos = 0, len = 0;
	int			prev_x = 1, prev_y = 1;
	int			w, replace_mode = 0;
	char		prev_ch;
	int			code, size = TED_LSZ-1;
    bool		first_key = true;

    if	( defval )
    	strcpy(dest, defval);
    else
		*dest = '\0';
	prev_x = cx;
	prev_y = cy;

	set_color(0x8F);
    
//	prev_x = strlen(last_message) + 1;
//	prev_y = scr_rows();
    gotoxy(prev_x, prev_y);
    clreol();
    gotoxy(prev_x, prev_y);
	printw("%s", dest);
    gotoxy(prev_x, prev_y);
    refresh();

    store_keys = true;	// enable keyboard-buffer
    
	while ( c != TEDK_ENTER && c != TEDK_ESCAPE )	{
    	if	( (c = inkey()) != 0 )	{
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
					pos -= dev_input_remove_char(dest, pos-1);
					len = strlen(dest);
					}
				else
					beep();
				break;
			case TEDK_DELETE: 		// delete
				if	( pos < len )		{
					dev_input_remove_char(dest, pos);
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
                    byte	cstr[3];
                    int		remain;
                    byte	*buf;
                    
                    cstr[0] = c;
                    cstr[1] = '\0';
                    
					if	( replace_mode )	{
                        // overwrite mode
                        remain = strlen(dest+pos);
                        buf = (char *) malloc(remain+1);
                        strcpy(buf, dest+pos);
                        memcpy(dest+pos, cstr, count);
                        dest[pos+count] = '\0';

                        count = 1;

                        if	( buf[0] )	// not a '\0'
                            strcat(dest, buf+count);
                        free(buf);
                        }
                    else	{
                        // insert mode
                        remain = strlen(dest+pos);
                        buf = (char *) malloc(remain+1);
                        strcpy(buf, dest+pos);
                        memcpy(dest+pos, cstr, count);
                        dest[pos+count] = '\0';
                        strcat(dest, buf);
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
            refresh();
			}	// inkey() loop

        Application->ProcessMessages();
    	}
        
    store_keys = false;	// disable keyboard-buffer
    
	set_color(7);
    refresh();
    status_line("");
    
	return dest;
}

// returns the number of the rows of the window
int		TedBCB::scr_rows()
{
	TRect	r;

    r = canvas->ClipRect;
    return (r.Height()/fy) - 1;
}

// returns the number of the columns of the window
int		TedBCB::scr_cols()
{
	TRect	r;

    r = canvas->ClipRect;
    return (r.Width()/fx);
}

// just produce a beep
void	TedBCB::beep()
{
	MessageBeep(-1);
}

// print a char at current position
void	TedBCB::addch(int ch)
{
	printw("%c", ch);
}

// Alt+X
void	TedBCB::sys_exit()
{
	
}

//
void	TedBCB::push_key(int c, int mcas)
{
	int		tc[10], idx = 0;

	if	( !Enabled )
    	return;
        
    memset(&tc, 0, sizeof(int) * 10);

    // unselect
    if	( (mcas & 7) == 0 && has_selection() )	{
    	if	(  c == VK_LEFT || c == VK_HOME
    		|| c == VK_RIGHT || c == VK_END
    		|| c == VK_UP || c == VK_PRIOR
    		|| c == VK_DOWN || c == VK_NEXT
            ) {

            tc[idx] = TEDK_SELCANCEL;
            idx ++;
        	}
    	}
    
    // select
    if	( (mcas & 1) != 0 && !has_selection() )	{
    	if	(  c == VK_LEFT || c == VK_HOME
    		|| c == VK_RIGHT || c == VK_END
    		|| c == VK_UP || c == VK_PRIOR
    		|| c == VK_DOWN || c == VK_NEXT
            ) {

            tc[idx] = TEDK_SELMARK;
            idx ++;
        	}
    	}
    
	// translate key
    if	( mcas )	{
        switch ( c )	{
        case	VK_TAB:			tc[idx] = TEDK_TAB;		break;
//      case	VK_RETURN:		tc[idx] = TEDK_ENTER;	break;
//      case	VK_BACK:		tc[idx] = TEDK_BSPACE;	break;
        case	VK_ESCAPE:		tc[idx] = TEDK_ESCAPE;	break;
        case	VK_INSERT:		tc[idx] = /*TEDK_INSOVR*/ TEDK_PASTE;	break;
        case	VK_DELETE:
        	if	( has_selection() )
	        	tc[idx] = TEDK_DELSEL;	
            else
	        	tc[idx] = TEDK_DELCHAR;	
            break;
        case	VK_F10:
            if	( mcas & 0x4 )
            	tc[idx] = TEDK_NDC;
            else
            	tc[idx] = TEDK_COMMAND;
            break;
        case	VK_F1:
        	if	( mcas & 0x1 )
    	       	tc[idx] = TEDK_HELP;
            else
	           	tc[idx] = TEDK_HELPWORD;
            break;
        case	VK_F3:
        case	VK_F5:
        	tc[idx] = TEDK_FNEXT;
        	if	( mcas & 0x1 )
				tc[idx] = TEDK_FPREV;
            break;
        case	VK_NEXT:
            if	( mcas & 0x4 )
                tc[idx] = TEDK_EOF;
            else
                tc[idx] = TEDK_NEXTSCR;
            break;
        case	VK_PRIOR:
            if	( mcas & 0x4 )
                tc[idx] = TEDK_BOF;
            else
                tc[idx] = TEDK_PREVSCR;	
            break;
        case	VK_HOME:
        	// Brief's HOME
			if	( last_tc == TEDK_PREVSCR && row() == 0 && col() == 0 )	
				tc[idx] = TEDK_BOF;
            else if	( last_tc == TEDK_HOME && col() == 0 )
				tc[idx] = TEDK_PREVSCR;
			else
				tc[idx] = TEDK_HOME;	
            break;
        case	VK_END:
        	// Brief's END
			if	( last_tc == TEDK_NEXTSCR && row() == scr_rows() && col() == col_end() )
				tc[idx] = TEDK_EOF;
            else if	( last_tc == TEDK_EOL && col() == col_end() )
				tc[idx] = TEDK_NEXTSCR;
			else
	       		tc[idx] = TEDK_EOL;      
            break;
        case	VK_DOWN:
        	tc[idx] = TEDK_DOWN;		
            break;
        case	VK_LEFT:
        	tc[idx] = TEDK_LEFT;		
            break;
        case	VK_RIGHT:
        	tc[idx] = TEDK_RIGHT;	
            break;
        case	VK_UP:
       		tc[idx] = TEDK_UP;		
            break;
        default:
            if	( (c == 'L') && (mcas & 0x4) )
                tc[idx] = TEDK_REFRESH;
            else if	( (c == 'F') && (mcas & 0x4) )
                tc[idx] = TEDK_SEARCH;
            else if	( (c == 'P') && (mcas & 0x4) )
                tc[idx] = TEDK_FPREV;
            else if	( (c == 'N') && (mcas & 0x4) )
	        	tc[idx] = TEDK_FNEXT;
            else if	( (c == 'Z') && (mcas & 0x4) )
                tc[idx] = TEDK_UNDO;
            else if	( (c == 'Y') && (mcas & 0x4) )
				tc[idx] = TEDK_KILL;
            else if	( (c == 'L') && (mcas & 0x4) )
				tc[idx] = TEDK_DELEOL;	// delete to end-of-line
            else if	( (c == 'A') && (mcas & 0x4) )
				tc[idx] = TEDK_INDENT;
            else if	( (c == 'C') && (mcas & 0x4) )	{
            	if	( has_selection() )
                	tc[idx] = TEDK_COPYSEL;
                else
                	tc[idx] = TEDK_LINECOPY;
                }
            else if	( (c == 'V') && (mcas & 0x4) )
                tc[idx] = TEDK_PASTE;
            else if	( (c == 'B') && (mcas & 0x2) )
                tc[idx] = TEDK_BUFLIST;
            else if	( (c == 'M') && (mcas & 0x2) )
                tc[idx] = TEDK_SELMARK;
            else if	( (c == 'L') && (mcas & 0x2) )
                tc[idx] = TEDK_LINESELMARK;
            else if	( (c == 'O') && (mcas & 0x2) )
                tc[idx] = TEDK_CHANGEFN;
            else if	( (c == 'W') && (mcas & 0x2) )
                tc[idx] = TEDK_SAVE;
            else if	( (c == 'E') && (mcas & 0x2) )
                tc[idx] = TEDK_LOAD;
            else if	( (c == 'X') && (mcas & 0x4) )	{
            	if	( has_selection() )
	                tc[idx] = TEDK_CUTSEL;
                else
	                tc[idx] = TEDK_LINECUT;
                }
            else if	( (c == '1') && (mcas & 0x2) )
            	tc[idx] = TEDK_MARK1;
            else if	( (c == '2') && (mcas & 0x2) )
            	tc[idx] = TEDK_MARK2;
            else if	( (c == '3') && (mcas & 0x2) )
            	tc[idx] = TEDK_MARK3;
            else if	( (c == '4') && (mcas & 0x2) )
            	tc[idx] = TEDK_MARK4;
            else if	( (c == '5') && (mcas & 0x2) )
            	tc[idx] = TEDK_MARK5;
            else if	( (c == '6') && (mcas & 0x2) )
            	tc[idx] = TEDK_MARK6;
            else if	( (c == '7') && (mcas & 0x2) )
            	tc[idx] = TEDK_MARK7;
            else if	( (c == '8') && (mcas & 0x2) )
            	tc[idx] = TEDK_MARK8;
            else if	( (c == '9') && (mcas & 0x2) )
            	tc[idx] = TEDK_MARK9;
            else if	( (c == '0') && (mcas & 0x2) )
            	tc[idx] = TEDK_MARK0;
            else if	( (c == 'J') && (mcas & 0x2) )
            	tc[idx] = TEDK_GMARK;
            else if	( (c == 'I') && (mcas & 0x2) )
            	tc[idx] = TEDK_INSOVR;
            else if	( (c == 'G') && (mcas & 0x4) )
            	tc[idx] = TEDK_GOTO;
            else
            	tc[idx] = 0;
//          tc[idx] = (mcas << 8) | c;
            }; 
		}
	else if	( c == '\n' || c == '\r' )
    	tc[idx] = TEDK_ENTER;
	else if	( c == '\t' )
    	tc[idx] = TEDK_TAB;
	else if	( c == '\b' )
    	tc[idx] = TEDK_BSPACE;
	else if	( c < 32 )	// ignore
    	tc[idx] = 0;
    else if ( (c & 0xFF) == 0 )
    	tc[idx] = 0;
    else
    	tc[idx] = c;
        
//TEDK_DEFKEY,	// define key
//TEDK_SENSIT,	// case sensitivity switch
//TEDK_CHCASE,	// change case
//TEDK_MATCH,		// match
//TEDK_REPEAT,	// repeat ???
//TEDK_SUBST,		// substitute();
//TEDK_WORD,		// one word forward
//TEDK_DELWORD,	// delete next word
//TEDK_BWORD,		// one word backward
//TEDK_DELBWORD,	// delete back word
//TEDK_DELBOL,	// delete to start of line
//TEDK_AGAIN,		// repeat_again();
//TEDK_MENU,		// menu();
//TEDK_HELP,		// help(); 
//TEDK_CENTER,	// ???
//TEDK_ALIAS,		// ???
//TEDK_DELETE,

	for ( idx = 0; tc[idx]; idx ++ )	{
    	int		code = tc[idx];
        
        last_tc = code;
        if	( !store_keys )	{
			// execute
            manage_key(code);
            }
        else	{
        	// store it
            k_buf[k_head] = code;
            k_head ++;
            if	( k_head == 256 )
            	k_head = 0;
            } 
		}
        
	if	( !store_keys && has_selection() )		
		manage_key(TEDK_REFRESH);
	if	( !store_keys )	
		place_cursor();
} 

//
int		TedBCB::inkey()
{
	int		c;
    
	if	( k_head == k_tail )
    	return 0;
	c = k_buf[k_tail];
    k_tail ++;
    if	( k_tail == 256 )
    	k_tail = 0;
	return c;
}

//
void	TedBCB::update_cursor()
{
	int		x, y;

	lcx = cx;
    lcy = cy;
    
    // show cursor
    for ( y = lcy*fy; y < (lcy+1)*fy; y ++ )	{
        for ( x = lcx*fx; x < (lcx+1)*fx; x ++ )	
        	canvas->Pixels[x][y] = canvas->Pixels[x][y] ^ 0xFFFFFF;
        }
}

/**
*/
void	TedBCB::paint()
{
	if	( !Enabled )
    	return;
        
	lock_refresh = true;

	if	( bmp )
		delete bmp;
    
    bmp = new Graphics::TBitmap;
    bmp->Width = win->ClientRect.Width();
    bmp->Height = win->ClientRect.Height();
    bmp->Canvas->FillRect(win->ClientRect);
    canvas = bmp->Canvas;
	canvas->Font->Name = font_name;
    canvas->Font->Size = font_size;

    fx = canvas->TextWidth("Q");
    fy = canvas->TextHeight("Q");
    
 	adjust_screen(true);
    place_cursor();

//	img->Picture->Bitmap->Assign(bmp);
    update_cursor();
	img->Canvas->Draw(0,0,bmp);
    update_cursor();
	lock_refresh = false;
}

/**
*	flush screen
*/
void	TedBCB::refresh()
{
	if	( !Enabled )
    	return;
        
	if	( !lock_refresh )	{
	    //	img->Picture->Bitmap->Assign(bmp);
        update_cursor();
        img->Canvas->Draw(0,0,bmp);
        update_cursor();
        }
}

/**
*	setup the font
*/
void	TedBCB::set_font(const char *font, int size)
{
	strcpy(font_name, font);
    font_size = size;
    win->Invalidate();
}

/**
*	draw the status line
*/
void	TedBCB::status_line(const char *say)
{
	set_color(0x8F);
    
	gotoxy(0, scr_rows());
    clreol();
    if ( say )	
		printw(" %s ", say);

    set_color(7);
}

/**
*	editor asks user for input
*/
char*	TedBCB::ask(const char *prompt, char *buf, int size, const char *defval)
{
	AnsiString s;

/*
    s.printf("SBPad: %s", get_curr_title());
    s = InputBox(s, prompt, "");
    strncpy(buf, s.c_str(), size);
    buf[size-1] = '\0';
    return buf;
*/
	status_line(prompt);
	s = get_string(defval);
    strncpy(buf, s.c_str(), size);
    buf[size-1] = '\0';
    return buf;
}

/**
*	emulation of VGA16 text colors
*/
void	TedBCB::set_color(int color)
{
	#pragma warn -8006
	TColor	vga16[] = 
    	{
        0x000000,	// 0
        0xA00000,	// 1
        0x00A000,	// 2
        0xA0A000,	// 3
        0x0000A0,		// 4
        0xA000A0,	// 5
        0x00A0A0,	// 6
        0xA0A0A0,	// 7
        0x808080,		// 8
        0xFF0000,		// 9
        0x00FF00,		// 10
        0xFFFF00,		// 11
        0x0000FF,		// 12
        0xFF00FF,	// 13
        0x00FFFF,	// 14
        0xFFFFFF		// 15
    	};
	#pragma warn +8006

//	if	( color == TED_COMMAND_COLOR )	
//    	canvas->Font->Style = TFontStyles() << fsBold;
//	else
//    	canvas->Font->Style = TFontStyles();
        
   	canvas->Font->Color  = vga16[color & 0xF];
	canvas->Brush->Color = vga16[(color & 0xF0) >> 4];
}

/**
*	editor's notification: copy to clipboard
*/
void	TedBCB::clip_copy()
{
	char	*text;

    text = get_clip();
	Clipboard()->Open();
    Clipboard()->SetTextBuf(text);
	Clipboard()->Close();
    free(text);
}

/**
*	editor's notification: paste from clipboard
*/
void	TedBCB::clip_paste()
{
	AnsiString	s;

	if ( Clipboard()->HasFormat(CF_TEXT) )	{
		s = Clipboard()->AsText;
		set_clip(s.c_str());
        }
}

