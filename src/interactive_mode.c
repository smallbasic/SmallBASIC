/**
*	SmallBASIC, console editor (interactive mode)
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*
*	Nicholas Christopoulos
*/

#include "sbapp.h"
#include <stdio.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/stat.h>
#if defined(_DOS)
	#include <conio.h>
#elif defined(_UnixOS)
	#include <sys/ioctl.h>
#endif
#include "help_subsys.h"

#if defined(_DOS)
#define	CLS()	clrscr()
#else
#define	CLS()	printf("\033[2J")
#endif

#define	MIN(a,b)	( ((a) < (b)) ? (a) : (b) )

typedef char *	char_p;

// program list
typedef struct {
	int		num;
	int		mark;
	char	*buf;
	void	*next;
	} intm_t;

intm_t	*intm_head;

static int	intm_ansi = 0;
static int	intm_scr_w = 79, intm_scr_h = 24;
static int	intm_num_mode = 0;
static char *intm_cmd_line_hist;
#define	cmd_line_hist intm_cmd_line_hist
#define	num_mode	intm_num_mode
#define	use_ansi	intm_ansi
#define	scr_w		intm_scr_w
#define	scr_h		intm_scr_h

char	*intm_tab2spc(char *src);

/*
*	initialize interactive mode shell
*/
void	intm_init()
{
	intm_head = NULL;

	scr_w = 79;
	scr_h = 25;

#if defined(_UnixOS)
	#if defined(TIOCGWINSZ)
	{
	   	struct winsize ws;

	   	/* Get and check window size. */
		if ( ioctl(0, TIOCGWINSZ, &ws) >= 0 && ws.ws_row != 0 && ws.ws_col != 0 )  {
	       	scr_w = ws.ws_col;
	       	scr_h = ws.ws_row;
	       	}
	}
	#endif

	use_ansi =
		(strstr(getenv("TERM"), "linux") != NULL) || 
		(strstr(getenv("TERM"), "xterm") != NULL) || 
		(strstr(getenv("TERM"), "ansi") != NULL);
#endif
}

/* --- string lib -------------------------------------------------------------------------------------------------------- */

/**
*	next word
*/
const char	*intm_getword(const char *text, char *dest)
{
	char	*p = (char *) text;
	char	*d = dest;
	int		dp = 0;

	if	( p == NULL )	{
		*dest = '\0';
		return 0;
		}

	while ( is_space(*p) )	p ++;

	if	( *p == '\"' )	{
		dp = !dp;
		p ++;
		}
	if	( !dp )	{
		// no quoted
		while ( is_alnum(*p) || (*p == '_') )	{
			*d = to_upper(*p);
			d ++; p ++;
			}
		}
	else	{
		// quoted
		while ( *p && *p != '\"' )	{
			*d = *p;
			d ++; p ++;
			}

		if	( *p == '\"' )
			p ++;
		}

	*d = '\0';
	while ( is_space(*p) )	p ++;

	// special case, something wrong, jump to next char
	if	( p == text )	{
		*dest = *p;
		*(dest+1) = '\0';
		p ++;
		while ( is_space(*p) )	p ++;
		}

	return p;
}

/*
*	removes spaces and returns a new string
*/
char	*intm_trimdup(const char *str)
{
	char	*buf;
	char	*p;

	buf = (char *) malloc(strlen(str)+1);
	strcpy(buf, str);

	if	( *str == '\0' )	
		return buf;
		
	p = (char *) str;
	while ( is_wspace(*p) )		p ++;
	strcpy(buf, p);

	if	( *p != '\0' )	{
		p = buf;
		while ( *p )	p ++;
		p --;
		while ( p > buf && is_wspace(*p) )	p --;
		p ++;
		*p = '\0';
		}
	return buf;			
}

/*
*	true if the word is integer
*/
int		intm_is_integer(const char *src)
{
	const char *p = src;

	while ( *p == ' ' || *p == '\t' )	p ++;
	while ( *p )	{
		if	( !isdigit(*p) )
			return 0;
		if	( *p == ' ' || *p == '\t' )
			break;
		p ++;
		}
	return 1;
}

/*
*	beautify code line
*/
void	intm_beautify(char *src)
{
	char	*dest, *d;
	char	*p;
	int		dq = 0;

	dest = strdup(src);
	p = src; d = dest;
	while ( *p == ' ' || *p == '\t' )	p ++;

	while ( *p )	{
		if	( *p == '\"' )	{
			dq = !dq;
			*d ++ = *p;
			}
		else if ( !dq )	{
			if	( *p == ' ' || *p == '\t' )	{
				*d ++ = ' ';
				while ( *(p+1) == ' ' || *(p+1) == '\t' )	p ++;
				}
			else
				*d ++ = to_upper(*p);
			}
		else
			*d ++ = *p;

		p ++;
		}

	*d = '\0';
	strcpy(src, dest);
}

/*
*	convert tabs to spaces
*/
char	*intm_tab2spc(char *src)
{
	int		col = 1, tabsize=4;
	int		new_col, i;
	char	*p;
	char	*dst, *d;

	p = src;
	d = dst = malloc(strlen(src)*tabsize+1);

	while ( *p )	{
		switch	( *p )	{
		case	'\t':
			new_col = ((col / tabsize) + 1) * tabsize;
			for ( i = col; i <= new_col; i ++ )	{
				*d = ' ';
				d ++;
				col ++;
				}
			break;
		case	'\n': case	'\r':
			col = 1;
			// no break
		default:
			*d = *p;
			col ++;
			d ++;
			}

		p ++;
		}

	*d = '\0';
	return dst;
}

/*
*/
char	*strndup(const char *src, int size)
{
	char	*p;
	
	p = (char *) malloc(size+1);
	strncpy(p, src, size);
	p[size] = '\0';
	return p;
}

/* */
void	intm_file_beutify(char *name)
{
	unsigned char	*n, *r, *p;
	
	n = intm_trimdup(name);
	p = n;
	while ( *p )	{
		if	( *p == '\"' || *p == '\'' || *p < ' ' )
			*p = ' ';
		p ++;
		}
	r = intm_trimdup(n);
	strcpy(name, r);
	free(n);
	free(r);
}
/* --- screen fx --------------------------------------------------------------------------------------------------------- */

/*
*	print right-adjusted
*/
#define	rprintf		intm_rprintf
void	intm_rprintf(const char *fmt, ...)
{
	va_list	ap;
	char	msg[2048];

	va_start(ap, fmt);
	#if defined(_DOS)
	vsprintf(msg, fmt, ap);
	#else
	vsnprintf(msg, 2048, fmt, ap);
	#endif
	va_end(ap);

	if	( use_ansi )
		printf("\033[%dG%s", (int) (scr_w - strlen(msg)) + 1, msg);
	else
		printf("%s", msg);
}

/*
*	printf at...
*/
void	pratf(int x, int y, const char *fmt, ...)
{
	va_list	ap;
	char	msg[2048];

	#if defined(_DOS)
	gotoxy(y, x);
	#else
	if	( use_ansi )
		printf("\033[%d;%dH", y, x);
	#endif
	va_start(ap, fmt);
	#if defined(_DOS)
	vsprintf(msg, fmt, ap);
	#else
	vsnprintf(msg, 2048, fmt, ap);
	#endif
	va_end(ap);
	printf("%s", msg);
}

/*
*	output header
*/
#define	header	intm_header
void	intm_header(const char *fmt, ...)
{
	va_list	ap;
	char	msg[2048];
	int		i;

	va_start(ap, fmt);
	#if defined(_DOS)
	vsprintf(msg, fmt, ap);
	#else
	vsnprintf(msg, 2048, fmt, ap);
	#endif
	va_end(ap);

	if	( use_ansi )	{
		printf("\033[1m\033[30m");
		for ( i = 0; i < scr_w; i ++ )		printf("_");
		printf("\033[%dG %s", (int) (scr_w - strlen(msg)), msg);
		printf("\033[0m");
		printf("\n\n");
		}
	else
		printf("[%s]\n\n", msg);
}

/*
*	output footer
*/
#define	footer	intm_footer
void	intm_footer(const char *fmt, ...)
{
	va_list	ap;
	char	msg[2048];
	int		i;

	va_start(ap, fmt);
	#if defined(_DOS)
	vsprintf(msg, fmt, ap);
	#else
	vsnprintf(msg, 2048, fmt, ap);
	#endif
	va_end(ap);

	if	( use_ansi )	{
		printf("\033[1m\033[30m");
		for ( i = 0; i < scr_w; i ++ )		printf("_");
		printf("\n");
		rprintf(msg);
		printf("\033[0m");
		printf("\n");
		}
	else	
		printf("--- %s\n", msg);
}

/*
*	getkey/inkey
*/
#define	getkey	intm_getkey
int		intm_getkey(int waitf)
{
#if defined(_DOS)
	int		c = 0;

	if	( waitf )	{
		if	( (c = getch()) == 0 )	c = getch() | 0x100;
		}
	else	{
		if	( kbhit() )	{
			if	( (c = getch()) == 0 )	c = getch() | 0x100;
			}
		}
	return c;
#else // defined(_UnixOS)
	fd_set		rfds;
	struct timeval tv;
	int			ival;
	unsigned char	c = 0;
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
		tv.tv_usec=0;
		ival = select(1, &rfds, NULL, NULL, &tv);
		if	( ival )	{
			read(0, &c, 1);
			break;
			}
		} while ( c == 0 && waitf );

	//
	tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
	return c;
#endif
}

/*
*	getkey with prompt and range check
*/
#define	ask	intm_ask
char	intm_ask(const char *prompt, const char *valid)
{
	int		ch;

	if	( use_ansi )
		pratf(1, scr_h, "\033[1m\033[30m>>> %s (%s) ? ", prompt, valid);
	else	
		printf("? %s (%s) ? ", prompt, valid);

	do	{
		ch = getkey(1);
		if	( ch == '/' )					ch = '\1';
		if	( ch == '\n' )					ch = 'y';
		if	( ch == 27 )	{
			if	( strchr(valid, 'q') )
				ch = 'q';
			else
				ch = 'n';
			}
		} while ( strchr(valid, ch) == NULL );

	if	( use_ansi )
		printf("\r\033[K");
	else
		printf("\n");
	return ch;
}

/**
*	output header, used inside the output 
*/
#define	mheader	intm_midheader
void	intm_midheader(const char *fmt, ...)
{
	va_list	ap;
	char	msg[2048];
	int		i;

	va_start(ap, fmt);
	#if defined(_DOS)
	vsprintf(msg, fmt, ap);
	#else
	vsnprintf(msg, 2048, fmt, ap);
	#endif
	va_end(ap);

	if	( use_ansi )	{
		printf("\033[1m\033[30m");
		for ( i = 0; i < scr_w; i ++ )		printf("-");
		printf("\033[%dG %s", (int) (scr_w - strlen(msg)), msg);
		printf("\033[0m");
		printf("\n");
		}
	else	{
		for ( i = 0; i < (scr_w - strlen(msg)) - 1; i ++ )		printf("-");
		printf(" %s", msg);
		printf("\n");
		}
}

/*
*	display a multi-line text
*/
#define	less	intm_less
int		intm_less(const char *src, const char *title)
{
	int		line, termline, stop, len, quit = 0;
	char	*buf, *next, *cur;
	char	*fs;

	line = 0;
	termline = 2;
	stop = 0;
	cur = (char *) src;
	while ( cur )	{
		// build buf
		next = strchr(cur, '\n');
		
		if	( next )	{
			next ++;
			len = next - cur;
			buf = malloc(len+1);
			strncpy(buf, cur, len);
			buf[len] = '\0';
			cur = next;
			}
		else	{
			buf = strdup(cur);
			cur = NULL;
			}

		//
		fs = intm_tab2spc(buf);
		free(buf);

		if	( (int) strlen(fs) > scr_w - 8 )	{
			// split line
			int		blocks, i, w;
			char	*part;

			w = scr_w - 8;
			part = (char *) malloc(w+1);
			blocks = (strlen(fs) / w) + 1;

			for ( i = 0; i < blocks; i ++ )	{
				// print left part
				if	( use_ansi )	{
					if	( i == 0 )	
						printf("\033[1m\033[30m%5d:\033[0m ", ++ line);
					else
						printf("\033[1m\033[30m%5s:\033[0m ", "...");
					}
				else	{
					if	( i == 0 )	
						printf("%5d: ", ++ line);
					else
						printf("%5s: ", "...");
					}

				// print string-segment
				strncpy(part, fs + i*w, w);
				part[w] = '\0';
				printf("%s", part);
				if	( i != blocks-1 )
					printf("\n");
						
				termline ++;

				if	( termline > scr_h - 2 )	{
					termline = 1;
					switch ( ask("more", "q/y/n") )	{
					case	'q':
						quit = stop = 1;
					case	'n':
						stop = 1;
						break;
					default:
						mheader("'%s' at %d", title, line);
						}
					}

				//
				if	( stop || quit )	break;
				}

			free(part);
			}
		else	{
			if	( use_ansi )
				printf("\033[1m\033[30m%5d:\033[0m %s", ++ line, fs);
			else
				printf("%5d: %s", ++ line, fs);

			termline ++;
			}

		free(fs);

		//
		if	( termline > scr_h - 2 )	{
			termline = 1;
			switch ( ask("more", "q/y/n") )	{
			case	'q':
				quit = stop = 1;
			case	'n':
				stop = 1;
				break;
			default:
				mheader("'%s' at %d", title, line);
				}
			}

		//
		if	( stop || quit )	break;
		if	( !cur )
			printf("\n");
		}

	return !quit;
}

/*
*	message 'information' type
*/
#define	info	intm_info
void	intm_info(const char *fmt, ...)
{
	va_list	ap;
	char	msg[2048];

	va_start(ap, fmt);
	#if defined(_DOS)
	vsprintf(msg, fmt, ap);
	#else
	vsnprintf(msg, 2048, fmt, ap);
	#endif
	va_end(ap);

	if	( use_ansi )
		printf("\033[0m\033[32m(i) %s\033[0m\n", msg);
	else
		printf("%s\n", msg);
}

/*
*	message 'error' type
*/
#define	errfmt	intm_errfmt
void	intm_errfmt(const char *fmt, ...)
{
	va_list	ap;
	char	msg[2048];

	va_start(ap, fmt);
	#if defined(_DOS)
	vsprintf(msg, fmt, ap);
	#else
	vsnprintf(msg, 2048, fmt, ap);
	#endif
	va_end(ap);

	if	( use_ansi )
		printf("\033[0m\033[31m* %s *\033[0m\n", msg);
	else
		printf("%s\n", msg);
}

/*
*/
#define	sysfmt	intm_sysfmt
void	intm_sysfmt(const char *fmt, ...)
{
	va_list	ap;
	char	msg[2048];

	va_start(ap, fmt);
	#if defined(_DOS)
	vsprintf(msg, fmt, ap);
	#else
	vsnprintf(msg, 2048, fmt, ap);
	#endif
	va_end(ap);

	system(msg);
}

/* --- basic style editing ----------------------------------------------------------------------------------------------- */

/*
*	add program line
*/
void	intm_add_line(int line, const char *text)
{
	intm_t	*np, *cp, *pp;

	if	( line <= 0 )
		return;

	np = (intm_t *) malloc(sizeof(intm_t));
	np->num = line;
	np->buf = strdup(text);
	np->next = NULL;

	if ( !intm_head )	// the list is empty
		intm_head = np;
	else	{
		// find the correct position
		cp = pp = intm_head;
		while ( cp )	{
			if	( cp->num >= np->num )
				break;
			pp = cp;
			cp = cp->next;
			}

		// replace line
		if	( cp && cp->num == np->num )	{
			free(cp->buf);
			cp->buf = np->buf;
			free(np);
			}
		else	{	// connect it
			if	( !cp )	// add at the end
				pp->next = np;
			else	{
				if	( pp == intm_head && cp == intm_head )	{	
					// add at beginning
					np->next = intm_head;
					intm_head = np;
					}
				else	{
					// add somewhere in the middle
					np->next = pp->next;
					pp->next = np;
					}
				}
			}
		}
}

/*
*	display program list
*/
void	intm_list(int pc, char_p *pt)
{
	intm_t	*cp;
	int		start = -1;
	int		end = -1;

	if	( pc > 1 )	{
		if ( pc == 2 )
			start = end = atoi(pt[1]);
		else if	( pc == 3 && pt[1][0] == '-' )	{
			start = -1;
			end   = atoi(pt[2]);
			}
		else if ( pc == 3 && pt[2][0] == '-' )	{
			start = atoi(pt[1]);
			end = -1;
			}
		else if ( pc == 4 && pt[2][0] == '-' )	{
			start = atoi(pt[1]);
			end = atoi(pt[3]);
			}
		else	{
			errfmt("SYNTAX ERROR");
			return;
			}
		}

	// list
	cp = intm_head;
	while ( cp )	{
		if	( end != -1 )	{
			if	( cp->num > end )
				break;
			}

		if	( cp->num >= start )	
			printf("%4d: %s\n", cp->num, cp->buf);

		cp = cp->next;
		}
}

/*
*	renums the program lines
*/
void	intm_renum(int pc, char_p *pt)
{
	intm_t	*cp;
	int		start = -1;
	int		step = 10;
	int		count = 0;

	if	( pc > 3 )	{
		start = atoi(pt[1]);
		step = atoi(pt[3]);
		}
	else if	( pc > 1 )	
		step = atoi(pt[1]);

	cp = intm_head;
	while ( cp )	{
		if	( cp->num >= start )	{
			count += step;
			cp->num = count;
			}
		cp = cp->next;
		}
}

/*
*	returns the maximum line number in the program list
*/
int		intm_get_maxline()
{
	intm_t		*cp;
	int			n = 0;

	cp = intm_head;
	while ( cp )	{
		n = cp->num;
		cp = cp->next;
		}
	return n;
}

/*
*	kill program list
*/
void	intm_clear_list()
{
	intm_t		*np, *pp;

	pp = np = intm_head;
	while ( np )	{
		free(np->buf);
		pp = np;
		np = np->next;
		free(pp);
		}
	intm_head = NULL;
}

/*
*	erase program lines
*/
void	intm_erase(int pc, char_p *pt)
{
	intm_t	*cp, *pp, *ep;
	int		start = -1;
	int		end = -1;

	if	( pc > 1 )	{
		if ( pc == 2 )
			start = end = atoi(pt[1]);
		else if	( pc == 3 && pt[1][0] == '-' )	{
			start = -1;
			end   = atoi(pt[2]);
			}
		else if ( pc == 3 && pt[2][0] == '-' )	{
			start = atoi(pt[1]);
			end = -1;
			}
		else if ( pc == 4 && pt[2][0] == '-' )	{
			start = atoi(pt[1]);
			end = atoi(pt[3]);
			}
		else	{
			errfmt("SYNTAX ERROR");
			return;
			}
		}

	// kill
	cp = pp = intm_head;
	while ( cp )	{
		if	( end != -1 )	{
			if	( cp->num > end )
				break;
			}

		if	( cp->num >= start )	{
			//printf("%4d: %s\n", cp->num, cp->buf);
			if	( cp == intm_head )	{
				ep = cp;
				intm_head = cp = cp->next;
				free(ep->buf);
				free(ep);
				}
			else	{
				pp->next = cp->next;
				ep = cp;
				cp = cp->next;
				free(ep->buf);
				free(ep);
				}
			}
		else	{
			pp = cp;
			cp = cp->next;
			}
		}
}

/*
*	save SB code
*/
void	intm_save(int pc, char *file)
{
	intm_t	*cp;
	FILE	*fp;
	char	fname[1024];
	int		count = 0;

	if	( pc != 2 )	{
		errfmt("SYNTAX ERROR");
		return;
		}

	// write
	strcpy(fname, file);
	intm_file_beutify(fname);
	if	( !strchr(fname, '.') )	strcat(fname, ".sb");
	
	fp = fopen(fname, "wt");
	if	( !fp )	{
		errfmt("I/O ERROR [%s: %s]", fname, strerror(errno));
		return;
		}

	cp = intm_head;
	while ( cp )	{
		count ++;
		fprintf(fp, "%s\n", cp->buf);
		cp = cp->next;
		}

	fclose(fp);
	info("%s: %d lines saved", fname, count);
}

/*
*	load or merge sb code
*/
int		intm_load(int pc, const char *file, int merge)
{
	FILE	*fp;
	char	buf[4096], *p;
	int		line, step, count = 0;
	char	fname[1024];

	// read
	strcpy(fname, file);
	intm_file_beutify(fname);
	if	( !strchr(fname, '.') )	strcat(fname, ".sb");

	fp = fopen(fname, "rt");
	if	( !fp )	{
		errfmt("I/O ERROR [%s: %s]", fname, strerror(errno));
		return 0;
		}
	
	CLS();

	line = step = 10;
	if	( !merge )	// not MERGE
		intm_clear_list();	// clear list
	else	{
		line = merge;
		if	( line < 1 )
			line = 10;
		}

	while ( fgets(buf, 4096, fp) )	{
		count ++;

		// remove CR/LF
		if	( buf[strlen(buf)-1] == '\n' )
			buf[strlen(buf)-1] = '\0';

		intm_add_line(line, buf);

		// next number
		line += step;
		}

	fclose(fp);

	info("%s: %d lines loaded", fname, count);
	return 1;
}

/* --- shell commands ---------------------------------------------------------------------------------------------------- */

/* change directory */
void	intm_chdir(int pc, char *str)
{
	if	( pc > 1 )	{
		if	( chdir(str) )
			errfmt("CD FAILED");
		}
	else if ( pc == 1 )	{
		char	buf[1024];

		getcwd(buf, 1024);
		info("Current directory: [%s]", buf);
		}
	else
		errfmt("SYNTAX ERROR");
}

/*
*/
void	intm_dirwalk(const char *dir, const char *wc, int inside, int (*f)(const char *))
{
    char name[OS_PATHNAME_SIZE];
	struct dirent	*dp;
    DIR 	*dfd;
	struct	stat	st;
	int		callusr, contf = 1;
	
	if ((dfd = opendir(dir)) == NULL) {
		errfmt("CANNOT OPEN '%s'", dir);
        return;
	    }

    while ((dp = readdir(dfd)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue; /* skip self and parent */
        if ( strlen(dir)+strlen(dp->d_name)+2 > sizeof(name) )
			errfmt("NAME %s/%s IS TOO LING", dir, dp->d_name);
        else {
            sprintf(name, "%s%c%s", dir, OS_DIRSEP, dp->d_name);
			#if defined(_DOS)
			strlower(name);
			#endif

			/* check filename */
			if	( !wc )
				callusr = 1;
			else	
				callusr = wc_match(wc, dp->d_name);

			//
			if	( callusr )		contf = f(name);
			if	( !contf )		break;

			/*	proceed to the next */
			if	( access(name, R_OK) == 0 )	{
				stat(name, &st);
				if	( st.st_mode & S_IFDIR )	{
					if	( inside )
						intm_dirwalk(name, wc, 1, f);
					}
				}
        	}
	    }
    closedir(dfd);
}

/* display a file-name */
int		intm_print_fname(const char *file)
{
	char	*s, *ext;
	struct stat	st;
	char	clr[32], clrrst[32];
	char	size[64], tmbuf[64];
	char	attr[64];

	if	( (s = strrchr(file, '/')) != 0 )	
		s ++;
	else
		s = file;

	strcpy(clr, "");
	if	( use_ansi )	
		strcpy(clrrst, "\033[0m");
	else
		strcpy(clrrst, "");
	ext = strrchr(s, '.');

	stat(file, &st);

	// type
	if	( S_ISDIR(st.st_mode) )	{
		if	( use_ansi )
			strcpy(clr, "\033[1m\033[34m<dir>");
		else
			strcpy(clr, "<dir>");
		}
	else if	( S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) )	{
		if	( use_ansi )
			strcpy(clr, "\033[1m\033[35m<dev>");
		else
			strcpy(clr, "<dev>");
		}
#if defined(_UnixOS)
	else if	( S_ISLNK(st.st_mode) )	{
		if	( use_ansi )
			strcpy(clr, "\033[1m\033[37m(lnk)");
		else
			strcpy(clr, "(lnk)");
		}
#endif
	else if	( st.st_mode & S_IXUSR )	{
		if	( use_ansi )
			strcpy(clr, "\033[1m\033[32m(exe)");
		else
			strcpy(clr, "(exe)");
		}
	else if ( use_ansi )	{
		if ( ext )	{
			if	( 
				(strcmp(ext, ".c") == 0) ||
				(strcmp(ext, ".h") == 0) ||
				(strcmp(ext, ".hpp") == 0) ||
				(strcmp(ext, ".cxx") == 0) ||
				(strcmp(ext, ".cpp") == 0) ||
				(strcmp(ext, ".bas") == 0) ||
				(strcmp(ext, ".sb") == 0) ||
				(strcmp(ext, ".pl") == 0) ||
				(strcmp(ext, ".pas") == 0) ||
				(strcmp(ext, ".py") == 0) ||
				(strcmp(ext, ".asm") == 0) ||
				(strcmp(ext, ".java") == 0)
				)	{
				strcpy(clr, "\033[1m\033[36m     ");
				}
			else if	( 
				(strcmp(ext, ".gz") == 0) ||
				(strcmp(ext, ".zip") == 0) ||
				(strcmp(ext, ".bz") == 0) ||
				(strcmp(ext, ".bz2") == 0) ||
				(strcmp(ext, ".tgz") == 0)
				)	{
				strcpy(clr, "\033[31m     ");
				}
			else
				strcpy(clr, "     ");
			}
		else
			strcpy(clr, "     ");
		}
	else
		strcpy(clr, "     ");

	// size
	if	( st.st_size > (1024 * 1024 * 1024) )	
		sprintf(size, "%4ld GB   ", (st.st_size + (1024*1024*512)) / (1024 * 1024 * 1024) );
	else if	( st.st_size > (1024 * 1024) )	
		sprintf(size, "%4ld MB   ", (st.st_size + (1024*512)) / (1024 * 1024) );
	if	( st.st_size > 1024 )	
		sprintf(size, "%4ld kB   ", (st.st_size + 512) / 1024 );
	else	
		sprintf(size, "%4ld Bytes", st.st_size);

	// time
	strftime(tmbuf, 63, "%a %d %b %Y", localtime(&st.st_mtime));

	// rights
	strcpy(attr, "");
	strcat(attr, (st.st_mode & S_IRUSR) ? "r" : "-");
	strcat(attr, (st.st_mode & S_IWUSR) ? "w" : "-");
	strcat(attr, (st.st_mode & S_IXUSR) ? "x" : "-");
	strcat(attr, (st.st_mode & S_IRGRP) ? "r" : "-");
	strcat(attr, (st.st_mode & S_IWGRP) ? "w" : "-");
	strcat(attr, (st.st_mode & S_IXGRP) ? "x" : "-");
	strcat(attr, (st.st_mode & S_IROTH) ? "r" : "-");
	strcat(attr, (st.st_mode & S_IWOTH) ? "w" : "-");
	strcat(attr, (st.st_mode & S_IXOTH) ? "x" : "-");

	// print
	printf("%s%*s%s %s  %s  %s\n", clr, MIN(scr_w / 2, 32), s, clrrst, size, tmbuf, attr);
	return 1;	// success - continue
}

/* print the list of files */
void	intm_dir(int pc, const char *buf)
{
	if ( pc == 1 )	
		intm_dirwalk(".", "*", 0, intm_print_fname);
	else 
		intm_dirwalk(".", buf, 0, intm_print_fname);
}

/* */
int		intm_print_dir_name(const char *file)
{
	struct stat	st;
	
	stat(file, &st);
	if	( S_ISDIR(st.st_mode) )
		intm_print_fname(file);
	return 1;
}

/* print the list of files */
void	intm_dird(int pc, const char *buf)
{
	if ( pc == 1 )	
		intm_dirwalk(".", "*", 0, intm_print_dir_name);
	else 
		intm_dirwalk(".", buf, 0, intm_print_dir_name);
}

/* */
int		intm_print_exe_name(const char *file)
{
	struct stat	st;
	
	stat(file, &st);
	if	( (st.st_mode & S_IXUSR) && (!S_ISDIR(st.st_mode)) ) 
		intm_print_fname(file);
	return 1;
}

/* print the list of files */
void	intm_dire(int pc, const char *buf)
{
	if ( pc == 1 )	
		intm_dirwalk(".", "*", 0, intm_print_exe_name);
	else 
		intm_dirwalk(".", buf, 0, intm_print_exe_name);
}

/* */
int		intm_print_bas_name(const char *file)
{
	char	*ext;
	
	ext = strrchr(file, '.');
	if	( ext )	{
		if	( strcmp(ext, ".sb") == 0 || strcmp(ext, ".bas") == 0 )
			intm_print_fname(file);
		}
	return 1;
}

/* print the list of files */
void	intm_dirb(int pc, const char *buf)
{
	if ( pc == 1 )	
		intm_dirwalk(".", "*", 0, intm_print_bas_name);
	else 
		intm_dirwalk(".", buf, 0, intm_print_bas_name);
}

/* --- interface extras -------------------------------------------------------------------------------------------------- */

/*
*/
char	*intm_ascf(const char *src)
{
	char	*p;
	char	*dest, *d;

	p = (char *) src;
	dest = (char *) malloc(strlen(src) * 6 + 1);
	d = dest;

	while ( *p )	{
		if	( *p == '$' )	{
			if	( strncmp(p, "$b$", 3) == 0 )	// bold
				{ if ( use_ansi ) { strcpy(d, "\033[1m");  d += 4; } p += 3; }
			if	( strncmp(p, "$i$", 3) == 0 )	// italic
				{ if ( use_ansi ) { strcpy(d, "\033[36m"); d += 5; } p += 3; }
			if	( (strncmp(p, "$b-$", 4) == 0) || (strncmp(p, "$i-$", 4) == 0) )	// off
				{ if ( use_ansi ) { strcpy(d, "\033[0m"); d += 4; }  p += 4; }
			}
		else
			*d ++ = *p ++;
		}
	*d = '\0';

	return dest;
}

/*
*/
void	intm_help()
{
	static char *help_txt = 
	"$b$* SmallBASIC INTERACTIVE MODE COMMANDS *$b-$\n\n"
 	"$b$HELP$b-$ $i$[sb-keyword]$i-$\n"
		"Interactive mode help screen. The symbol '?' does the same."
		"\n\n"
	"$b$BYE$b-$\n" 
		"The BYE command ends SmallBASIC and returns the control to the Operating System."
		"\n\n"
	"$b$NEW$b-$\n"
		"The NEW command clears the memory and screen and prepares the computer for a new program. "
		"Be sure to save the program that you have been working on before you enter NEW as it is "
		"unrecoverable by any means once NEW has been entered."
		"\n\n"
	"$b$RUN$b-$ $i$[filename]$i-$\n"
		"The RUN command, which can also be used as a statement, starts program execution."
		"\n\n"
	"$b$CLS$b-$\n"
		"Clears the screen."
		"\n\n"
	"$b$LIST$b-$ $i${ [start-line] - [end-line] }$i-$\n"
		"The LIST command allows you to display program lines. "
		"If LIST is entered with no numbers following it, the entire program in memory is listed. "
		"If a number follows the LIST, the line with that number is listed. "
		"If a number followed by hyphen follows LIST, that line and all lines following it are listed. "
		"If a number preceeded by a hyphen follows LIST, all lines preceeding it and that line are listed. "
		"If two numbers separated by a hyphen follow LIST, the indicated lines and all lines between them are listed."
		"\n\n"
	"$b$RENUM$b-$ $i${ [initial-line] [,] [increment] }$i-$\n"
		"The RENUM command allows you to reassign line numbers."
		"\n\n"
	"$b$ERA$b-$ $i${ [start-line] - [end-line] }$i-$\n"
		"The ERA command allows you to erase program lines. "
		"If ERA is entered with no numbers following it, the entire program in memory is erased. "
		"If a number follows the ERA, the line with that number is erased. "
		"If a number followed by hyphen follows ERA, that line and all lines following it are erased. "
		"If a number preceeded by a hyphen follows ERA, all lines preceeding it and that line are erased. "
		"If two numbers separated by a hyphen follow ERA, the indicated lines and all lines between them are erased."
		"\n\n"
	"$b$NUM$b-$ $i$[initial-line [, increment]]$i-$\n"
		"The NUM command sets the values for the autonumbering. "
		"If the 'initial-line' and 'increment' are not specified, "
		"the line numbers start at 10 and increase in increments of 10."
		"\n\n"
	"$b$SAVE$b-$ $i$program-name$i-$\n"
		"The SAVE command allows you to copy the program in memory to a file. "
		"By using the LOAD command, you can later recall the program into memory."
		"\n\n"
	"$b$LOAD$b-$ $i$program-name$i-$\n"
		"The LOAD command loads 'program-name' file into memory. "
		"The program must first have been put on file using the SAVE command. "
		"LOAD removes the program currently in memory before loading 'program-name'." 
		"\n\n"
	"$b$MERGE$b-$ $i$program-name, line-number$i-$\n"
		"The MERGE command merges lines in 'program-name' file into the program lines already "
		"in the computer's memory. Use 'line-number' to specify the position where the lines "
		"will be inserted."
		"\n\n"
	"$b$* SHELL COMMANDS *$b-$\n\n"
	"$b$CD$b-$ $i$[path]$i-$\n"
		"Changed the current directory. Without arguments, displays the current directory."
		"\n\n"
	"$b$DIR$b-$ $i$[regexp]$i-$\n"
		"Displays the list of files. You can use DIRE for executables only or DIRD for directories only, or DIRB for BASIC sources."
		"\n\n"
	"$b$TYPE$b-$ $i$filename$i-$\n"
		"Displays the contents of the file."
		"\n\n"
	"$b$* EDITOR *$b-$\n\n"
	"I suggest to use an editor.\n"
	"Use [TAB] for autocompletion (re-edit program lines).\n"
	"Use [ARROWS] for history.\n"
	"There is no need to type line numbers, there will be inserted automagically if you use '+' in the beginning of the line.\n"
	"Line numbers are not labels, are used only for editing. Use keyword LABEL to define a label.\n"
	"Line numbers are not saved in files."
	;
	char	*ftext;
	
	header("help");
	ftext = intm_ascf(help_txt);
	less(ftext, "help");
	free(ftext);
	footer("SB shell");
}

/*
*	completition
*/
char    *intm_prg_gen(const char *text, int state)
{
	char	buf[4096], *pbuf;
	intm_t	*cp;
	static int len, idx, found;
	static char_p cmds[] = 
		{ "help", "bye", "new", "run", "cls",
			"list", "renum", "era", "num",
			"load", "save",
			"cd", "dir",
			"" 	};

	if	( !state )	{
		len = strlen(text);
		idx = 0;
		}

	if	( is_digit(text[0]) )	{
		cp = intm_head;
		found = 0;
		while ( cp )	{
			sprintf(buf, "%d %s", cp->num, cp->buf);
			if ( strncasecmp(buf, text, len) == 0 )	{
				found ++;
				if ( idx < found )	{
					idx ++;
					return strdup(buf);
					}
				}

			cp = cp->next;
			}
		}
/*
	else	{
		while ( cmds[idx][0] )	{	
			pbuf = cmds[idx];
			idx ++;
			if ( strncasecmp(pbuf, text, len) == 0 )	
				return strdup(pbuf);
			}
		}
*/
	return NULL;
}

/*
*	completition #2
*/
char **cons_comp(const char *text, int start, int end)
{
	char	**matches;

	matches = (char **)NULL;
	#if defined(_DOS) // djgpp
	matches = completion_matches(text, intm_prg_gen);
	#else
	matches = rl_completion_matches(text, intm_prg_gen);
	#endif

	return (matches);
}

/* get history */
char	*intm_get_hist()
{
	HIST_ENTRY	**the_list;
    int			i, size, pos;
	char		*s, *dest, *d;

	the_list = history_list ();
	size = 16;
	d = dest = (char *) malloc(size);
	if ( the_list )	{
		for ( i = 0; the_list[i]; i++ )	{
			s = (char *) (the_list[i]->line);

			pos  = d - dest;
			size += strlen(s) + 1;
			dest = (char *) realloc(dest, size);
			d = dest + pos;
			strcpy(d, s);
			d += strlen(s);
			*d ++ = '\n';
			}
		}
	*d = '\0';
	return dest;
}

/* set history */
void	intm_set_hist(const char *src)
{
	char	*p, *e, *np;
	int		size;

	clear_history();
	p = (char *) src;
	while ( (e = strchr(p, '\n')) )	{
		size = e - p;
		np = strndup(p, size);
		add_history(np);
		free(np);

		p = e;	p ++;
		}
}

/* code -> history */
void	intm_code_hist()
{
	intm_t	*cp;

	clear_history();

	cp = intm_head;
	while ( cp )	{
		add_history(cp->buf);
		cp = cp->next;
		}
}

/* */
void	intm_type(int pc, const char *orgfile)
{
	if	( pc > 1 )	{
		FILE	*fp;
		struct stat	st;
		char	*file;
		
		file = intm_trimdup(orgfile);
		intm_file_beutify(file);
		stat(file, &st);
		fp = fopen(file, "rt");
		if	( !fp )
			errfmt("CANNOT TYPE '%s'", file);
		else	{
			char	*buf;

			buf = (char *) malloc(st.st_size + 1);
			fread(buf, st.st_size, 1, fp);
			buf[st.st_size] = '\0';

			header("contents of '%s'", file);
			less(buf, file);
			footer("contents of '%s'", file);

			free(buf);
			fclose(fp);
			}
		free(file);
		}
	else
		errfmt("SYNTAX ERROR");
}

/* --- main function ----------------------------------------------------------------------------------------------------- */

/*
*/

// we must do something about this. any "law" info?
#define SB_STR_CPR	"Free Software Foundation, Nicholas Christopoulos"

int		interactive_mode(const char *fname)
{
	static char *buf = NULL, *parstr;
	char		*p;
	int			exitf = 0;
	int			line = 10, i, idx, step = 10;
	intm_t		*np, *pp;
	int			intm_argc;
	char_p		intm_argv[128];
	FILE		*fp;
	char		prompt[1024];

	intm_init();

	if	( !opt_quite )	{
		printf("SmallBASIC VERSION %s\n\tCopyright (c) 2000-2003 %s\n\n", SB_STR_VER, SB_STR_CPR);
		printf("Type 'HELP' for help; type 'BYE' or press Ctrl+C for exit.\n\n");
		}

	rl_readline_name = "sb";
	rl_attempted_completion_function = cons_comp;

	// create argument table
	for ( i = 0; i < 128; i ++ )
		intm_argv[i] = (char *) malloc(SB_SOURCELINE_SIZE+1);

	//
	intm_head = NULL;	
	num_mode = 0;

	// read .sbrc
	// ...

	// command loop
	printf("* READY *\n\n");
	do	{
		// read line
		if	( num_mode )	
			sprintf(prompt, "%5d ", line);
		else	{
			char	dir[1024];

			getcwd(dir, 1024);
			if	( use_ansi )
				sprintf(prompt, "\033[1m\033[34m%s\033[0m> ", dir);
			else
				sprintf(prompt, "%s > ", dir);
			//strcpy(prompt, "> ");
			}

		buf = readline(prompt);

		// 
		if	( buf && *buf )	{
			add_history(buf);

			// check lines
			intm_argc = 0;
			p = buf;
			*(p = (char *) intm_getword(p, intm_argv[intm_argc ++]));
			parstr = intm_trimdup(p);
			if	( *p )
				while ( *(p = (char *) intm_getword(p, intm_argv[intm_argc ++])) );

			// test args
/*
			printf("c=%d\n", intm_argc);
			for ( i = 0; i < intm_argc; i ++ )
				printf("[%s] ", intm_argv[i]);
*/

			// run command
			if	( intm_argc )	{
				if	( num_mode )	{
					intm_add_line(line, buf);
					line += step;
					}
				else	{
					// not - num-mode
					idx = 0;
					if	( intm_is_integer(intm_argv[idx]) )	{
						line = atoi(intm_argv[idx]);
						idx = 1;
						}

					if	( strcmp(intm_argv[0], "LIST") == 0 )	
						intm_list(intm_argc, intm_argv);
					else if	( strcmp(intm_argv[0], "RENUM") == 0 )
						intm_renum(intm_argc, intm_argv);
					else if	( strcmp(intm_argv[0], "LOAD") == 0 )	{
						if	( intm_load(intm_argc, parstr, 0) )
							line = intm_get_maxline() + step;
						}
					else if	( strcmp(intm_argv[0], "MERGE") == 0 )	{
						char	*p;

						p = strrchr(parstr, ',');
						if	( p )	{
							*p = '\0';
							p ++;
							if	( intm_load(intm_argc, parstr, atoi(p)) )
								line = intm_get_maxline() + step;
							}
						else
							errfmt("SYNTAX ERROR");
						}
					else if	( strcmp(intm_argv[0], "SAVE") == 0 )
						intm_save(intm_argc, parstr);
					else if	( 
						strcmp(intm_argv[0], "BYE") == 0 ||
						strcmp(intm_argv[0], "EXIT") == 0 ||
						strcmp(intm_argv[0], "QUIT") == 0 )
					
						exitf = -1;
					else if	( (strcmp(intm_argv[0], "HELP") == 0) || (intm_argv[0][0] == '?') )	{
						if	( intm_argc == 1 )	
							intm_help();
						else	{
							char 	*hs;
							char	*command;
							
							command = parstr;
							strlower(command);

							hs = help_getallinfo(command);
							if	( hs )	{
								header("'%s'", command);
								less(hs, "help");
								footer("SB shell");
								free(hs);
								}
							else	{
								// errfmt("HELP ON '%s' NOT FOUND", command);
								sysfmt("man %s", command);
								}
							}
						}
					else if	( strcmp(intm_argv[0], "CLS") == 0 )
						CLS();
					else if	( strcmp(intm_argv[0], "CD") == 0 )
						intm_chdir(intm_argc, parstr);
					else if	( strcmp(intm_argv[0], "DIR") == 0 )
						intm_dir(intm_argc, parstr);
					else if	( strcmp(intm_argv[0], "DIRD") == 0 )
						intm_dird(intm_argc, parstr);
					else if	( strcmp(intm_argv[0], "DIRE") == 0 )
						intm_dire(intm_argc, parstr);
					else if	( strcmp(intm_argv[0], "DIRB") == 0 )
						intm_dirb(intm_argc, parstr);
					else if	( strcmp(intm_argv[0], "TYPE") == 0 )
						intm_type(intm_argc, parstr);
					else if	( strcmp(intm_argv[0], "ERA") == 0 )
						intm_erase(intm_argc, intm_argv);
					else if	( strcmp(intm_argv[0], "NUM") == 0 )	{
						if	( intm_argc > 1 )	{
							line = atoi(intm_argv[1]);
							if	( line < 1 )
								line = 1;

							if	( intm_argc > 3 )		{
								step = atoi(intm_argv[3]);
								if	( step < 1 )
									step = 1;
								}
							}

						num_mode = 1;
						cmd_line_hist = intm_get_hist();
						intm_code_hist();
						}
					else if	( strcmp(intm_argv[0], "RUN") == 0 )	{
						int		errf = 0;

						if	( intm_argc > 1 ) {
							if	( (errf = !intm_load(intm_argc, parstr, 0)) == 0 )
								line = intm_get_maxline() + step;
							}

						// save
						if	( !errf )	{
							fp = fopen(fname, "wb");
							np = intm_head;
							while ( np )	{
								fprintf(fp, "%s\n", np->buf);
								np = np->next;
								}
							fclose(fp);

							// and run
							sbasic_main(fname);
							#if defined(_DOS)	// color bug (last-line, scroll)
							clreol();
							cprintf("\n");
							#endif
							}
						}
					else if	( strcmp(intm_argv[0], "NEW") == 0 )	{
						intm_clear_list();
						CLS(); 					// clear screen
						line = step = 10;		// autonumbering, first line = 10, step = 10
						}
					else if ( idx > 0 || intm_argv[0][0] == '+' )	{
						if	( idx == 0 )	{
							// no line number
							if ( intm_argv[idx][0] == '+' )	
								intm_add_line(line, parstr);
							}
						else	{
							// with line number
							p = buf;
							while ( strchr(" \t0123456789", *p) )	p ++;
							intm_add_line(line, p);
							}

						line += step;
						}
					else {
						// run tcsh
						p = buf; 
						while ( *p == ' ' || * p == '\t' )	p ++;
						system(p);
						}
					}
				}

			// final reset buffer
			free(parstr);
			free(buf);
			buf = NULL;
			}
		else if ( !buf )	// ctrl+c/d
			exitf = 1;
		else {	// empty line
			if	( num_mode )	{
				intm_set_hist(cmd_line_hist);
				free(cmd_line_hist);
				num_mode = 0;
				}
			}
		} while ( exitf == 0 );

	// clear argument list
	for ( i = 0; i < 128; i ++ )
		free(intm_argv[i]);

	// save list & clear
	if	( exitf != -1 )	{
		pp = np = intm_head;
		while ( np )	{
			free(np->buf);
			pp = np;
			np = np->next;
			free(pp);
			}

		return 1;
		}

	return 0;
}


