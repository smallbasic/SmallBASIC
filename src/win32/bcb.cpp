/*
*	BCB interface 
*
*	Nicholas Christopoulos
*/

#include "gmain.h"
#include "plist.h"
#include <stdio.h>
#include <stdarg.h>

#define	FONT_H		11		// window starting pos

//extern HANDLE	hStdFont;
//extern HANDLE	hBoldFont;

static int curline = 0;		// current line
static int curcol = 0;		// current column

static int fontheight = FONT_H;	// font height for stdFont/boldFont
static int maxline = (160/FONT_H) - 1;	// max. console line  ((160-15)/11 = 13.18 = 0..12)
static int useinvert = 0;	// use invert
static int useunderl = 0;	// use underline
static int tabsize = 32;	// tab size in pixels (160/32 = 5)

static TColor bgcolor = (TColor) 0x6fAfAf;

struct bcb_event {
	int		type;
	long	data;
	};

PtrList<bcb_event>	evlist;		// event list
Graphics::TBitmap	*scbmp;		// scaled output window (bitmap) 
Graphics::TBitmap	*bmp;		// output window (bitmap)
TCanvas				*screen;
TRect				rcFull, rcSCFull;
double				scmul = 2.0;

/*
*	refresh output window
*/
void	bcb_refresh()
{
	scbmp->Canvas->StretchDraw(rcSCFull, bmp);
	FMain->imgOutput->Picture->Assign(scbmp);
}

/*
*/
void	bcb_init()
{
	evlist.setAutoDelete(true);

	curline = curcol = 0;
	useinvert = 0;
	
	bmp = new Graphics::TBitmap();
	bmp->Width = 160;
	bmp->Height = 160;

	scbmp = new Graphics::TBitmap();
	scbmp->Width = 160 * scmul;
	scbmp->Height = 160 * scmul;
	
	FMain->pnlWRight->Width = scbmp->Width + 34;
	
	FMain->pnlOut->Width = scbmp->Width + 32;
	FMain->pnlOut->Height = scbmp->Height + 32;
	
	FMain->imgOutput->Left = 16;
	FMain->imgOutput->Top = 16;
	FMain->imgOutput->Width = scbmp->Width;
	FMain->imgOutput->Height = scbmp->Height;
	
	screen = bmp->Canvas;
	screen->Font->Name = "Small Fonts";
	screen->Font->Size = 7;
//	screen->Font->Handle = hBoldFont;
	screen->Pen->Color = (TColor) 0;
	screen->Pen->Mode = pmCopy;

	rcFull.Left = 0; rcFull.Top = 0;
	rcFull.Right = 160; rcFull.Bottom = 160;
	
	rcSCFull.Left = 0; rcSCFull.Top = 0;
	rcSCFull.Right = 160 * scmul; rcSCFull.Bottom = 160 * scmul;

	screen->Brush->Color = bgcolor;
	screen->FillRect(rcFull);

	bcb_refresh();
}	

/*
*/
void	bcb_close()
{
	bcb_refresh();
	evlist.clear();
	screen = NULL;
	delete bmp;
}

/*
*	memory manager error
*/
extern "C" void	bcb_mgrerr(char *fmt, ...)
{
	va_list	ap;
	char	buf[1024];

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	FMain->txtERR->Lines->Add(buf);
}
 
/*
*/
extern "C" void	bcb_doevents(int wait_flag)
{
	Application->ProcessMessages();
}

/**
*	next line
*/
void	bcb_nextln()
{
	Graphics::TBitmap	*tmp;
	int					my, dy;

	curcol = 0;

	if	( curline < maxline )	{
		curline ++;
		}
	else	{
		tmp = new Graphics::TBitmap();
		tmp->Width = 160;
		tmp->Height = 160;
		tmp->Canvas->Brush->Color = bgcolor;
		tmp->Canvas->FillRect(rcFull);
		my = maxline * fontheight;
		dy = fontheight;

		tmp->Canvas->CopyRect(
					TRect(0, 0, rcFull.Width(), my), 
					screen, 
					TRect(0, dy, rcFull.Width(), my + dy)
					);
					
		screen->Draw(0, 0, tmp);
		delete tmp;
		}
}

/*
*	calc next tab position
*/
int		bcb_calctab(int x)
{
	int		c = 1;

	while ( x > tabsize )	{
		x -= tabsize;
		c ++;
		}
	return c * tabsize;
}

/*
*	clear screen
*/
extern "C" void	bcb_cls(void)
{
	screen->FillRect(rcFull);
	curline = curcol = 0;
	bcb_refresh();
}

/**
*	Basic output
*
*	Supported control codes:
*	\t		tab (20 px)
*	\a		beep
*	\n		next line (cr/lf)
*	\xC		clear screen
*	\e[K	clear to end of line
*	\e[0m	reset all attributes to their defaults
*	\e[1m	set bold on
*	\e[4m	set underline on
*	\e[7m	reverse video
*	\e[21m	set bold off
*	\e[24m	set underline off
*	\e[27m	set reverse off
*/
void	bcb_base_write(const char *str)
{
	int		len, cx, escval, esccmd;
	byte	*p, tiv[2];
	TRect	r;
	TColor	prev_brush, text_color;

	len = StrLen(str);

	if	( len <= 0 )
		return;

	p = (byte *) str;
	while ( *p )	{
		switch ( *p )	{
		case	'\a':	// beep
			MessageBeep(-1);
			break;
		case	'\t':
			curcol = bcb_calctab(curcol);
			break;
		case	'\xC':
			bcb_cls();
			break;
		case	'\033':		// ESC ctrl chars (man console_codes)
			if	( *(p+1) == '[' )	{
				p += 2;

				escval = 0;
				esccmd = 0;

				if	( isdigit(*p) )	{
					escval = (*p - '0');
					p ++;

					if	( isdigit(*p) )	{
						escval = (escval * 10) + (*p - '0');
						p ++;
						}

					esccmd = *p;
					}
				else	
					esccmd = *p;

				switch ( esccmd )	{
				case	'K':
					r.Left = curcol;
					r.Top = curline * fontheight; 
					r.Right = 160;
					r.Bottom = fontheight; 
					screen->FillRect(r);
					break;
				case	'm':
					switch ( escval )	{
					case	0:	// reset
						screen->Font->Style = TFontStyles();
						useinvert = 0;
						break;
					case	1:	// set bold on
						screen->Font->Style = screen->Font->Style << fsBold;
						break;
					case	4:	// set underline on
//						screen->Font->Style = screen->Font->Style << fsUnderline;
						useunderl = 1;
						break;
					case	7:	// reverse video on
						useinvert = 1;
						break;
					case	21:	// set bold off
						screen->Font->Style = screen->Font->Style >> fsBold;
						break;
					case	24:	// set underline off
//						screen->Font->Style = screen->Font->Style >> fsUnderline;
						useunderl = 0;
						break;
					case	27:	// reverse video off
						useinvert = 0;
						break;
						};
					break;
					}
				}
			break;
		case	'\n':
			bcb_nextln();
			break;
		case	'\r':
			break;
		default:
			tiv[0] = *p;
			tiv[1] = '\0';
			
//			cx = 8; //FntCharWidth(*p);
			cx = screen->TextWidth(AnsiString((char *)tiv));
			if	( cx + curcol >= 160 )
				bcb_nextln();

			if	( useinvert )	{
				prev_brush = screen->Brush->Color;
				text_color = screen->Font->Color;
				screen->Brush->Color = (TColor) 0x0;
				screen->Font->Color = bgcolor;
				}
			screen->TextRect(
					TRect(curcol, curline*fontheight, curcol+cx, (curline+1)*fontheight),
					curcol, curline * fontheight, AnsiString( (char *) tiv ) );
			if	( useunderl )	{
				screen->Pen->Color = (useinvert) ? bgcolor : 0;
				screen->MoveTo(curcol-1, (curline+1) * fontheight-1);
				screen->LineTo(curcol+cx, (curline+1) * fontheight-1);
				}
			if	( useinvert )	{
				screen->Brush->Color = prev_brush;
				screen->Font->Color = text_color;
				}
			curcol += cx;
			};

		if	( *p == '\0' )
			break;
		p ++;
		}
}


/*
*	read a string from console
*/
extern "C" void bcb_gets(char *dest, int size)
{
}

/*
*	write a string to console
*/
extern "C" void	bcb_write(char *str)
{
	bcb_base_write(str);
	bcb_refresh();
}

/*
*	draw a line
*/
extern "C" void bcb_line(int x1, int y1, int x2, int y2, int color)
{
	if	( color >= 15 )
		screen->Pen->Color = bgcolor;
	else if ( color == 0 )
		screen->Pen->Color = (TColor) 0;
	else	
		screen->Pen->Color = (TColor) 0x404040;

	if	( x1 == x2 && y1 == y2 )	{	// pset
		SetPixel(screen->Handle, x1, y1, screen->Pen->Color);
		}
	else	{		
		SetPixel(screen->Handle, x1, y1, screen->Pen->Color);
		screen->MoveTo(x1, y1);
		screen->LineTo(x2, y2);
		}
	
	screen->Pen->Color = (TColor) 0;
	
	bcb_refresh();
}

/*
*	draw a rectangle (filled or not)
*/
extern "C" void	bcb_rect(RECT r, int color, int fill)
{
	TColor	old_brush = screen->Brush->Color;

	if	( color >= 15 )	{
		screen->Pen->Color = bgcolor;
		screen->Brush->Color = bgcolor;
		}
	else if ( color == 0 )	{
		screen->Pen->Color = (TColor) 0;
		screen->Brush->Color = (TColor) 0;
		}
	else	{
		screen->Pen->Color = (TColor) 0x404040;
		screen->Brush->Color = (TColor) 0x404040;
		}

	if	( fill )	{
		screen->FillRect(r);
		}
	else	{
		SetPixel(screen->Handle, r.right, r.top, screen->Pen->Color);
		screen->MoveTo(r.left, r.top);
		screen->LineTo(r.left, r.bottom);
		screen->MoveTo(r.left, r.top);
		screen->LineTo(r.right, r.top);
		
		SetPixel(screen->Handle, r.left, r.bottom, screen->Pen->Color);
		screen->MoveTo(r.right, r.bottom);
		screen->LineTo(r.left, r.bottom);
		screen->MoveTo(r.right, r.bottom);
		screen->LineTo(r.right, r.top);
		}
	
	screen->Pen->Color = (TColor) 0;
	screen->Brush->Color = old_brush;
	
	bcb_refresh();
}
