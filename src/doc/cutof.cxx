/*
*	cutof text-lines
*
*	Nicholas Christopoulos
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <pcre.h>

typedef char *	char_p;

#define OVECCOUNT 30    /* should be a multiple of 3 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// classic... text buffers 

// strdup using new[]
char	*pstrdup(const char *source)
{
	char	*ns = new char[strlen(source)+1];
	strcpy(ns, source);
	return ns;
}

// typical node (abstract class)
class node {
public:
	node	*next;
	node()	{ next = NULL; }
	virtual ~node() { }
	};

// typical list (abstract class)
class list : public node {
public:
	node	*head, *tail;

	list() : node()	{ head = tail = NULL; }
	virtual	~list()	{ clear(); }

	// clean up
	void	clear()	{
		node	*cp, *pp;

		cp = head;
		while ( cp )	{
			pp = cp;
			cp = cp->next;
			delete pp;
			}
		}

	// connect a new node
	void	connect(node *np)	{
		if	( !head )	
			head = tail = np;
		else
			( tail->next = np, tail = np );
		}
	};

// text node / text line
class tnode : public node {
public:
	char	*line;
	pcre	*re;
	int		type;
	char	*data;

	tnode(const char *src) : node()	{ line = pstrdup(src); type = 0; data = NULL; }
	virtual ~tnode()				{ delete[] line; if (data) delete[] data; }
	void	setData(const char *s)	{ if ( data ) delete[] data; data = pstrdup(s); }
	};

// text list
class tlist : public list {
public:
	tlist() : list()	{ }
	virtual ~tlist()	{ }

	// add a new line
	tnode	*add(const char *text)	{
		tnode	*np = new tnode(text);
		connect(np);
		return np;
		}
	};

// buffer list
class buflist : public list	{
public:
	buflist() : list()	{ }
	virtual ~buflist()	{ }

	// add a new buffer
	tlist	*add()	{
		tlist	*np = new tlist();
		connect(np);
		return np;
		}
	};

///////////////////////////////////////////////////////////////////////////////////////////////////////////

//buflist	*buffers;

tlist	ignstart;
tlist	ignstop;
tlist	ignline;
tlist	redir;

enum cltype { usr_strstr, use_pcre, use_re };

/* ignore empty lines more than this */
static int		ignlines = 1;

/* ignore block on/off */
static int		ignore = 0;

/* ignore control characters */
static int		ignctrl = 0;

// another panic routine
void	die(bool what, const char *fmt, ...)
{
	if	( what )	{
		char	buf[4096];
		va_list	ap;

		va_start(ap, fmt);
		vsnprintf(buf, 4096, fmt, ap);
		va_end(ap);
		perror(buf);
		exit(1);
		}
}

//														  
void	cleantext(const char *src, char *dst)
{
	const char	*p = src;
	char		*d = dst;

	while ( *p )	{
		if	( *p == '\b' )	{
			d --;
			p ++;
			}
		else	{
			if	( *p >= ' ' )
				*d ++ = *p ++;
			else
				p ++;
			}
		}
	*d = '\0';
}

//
void	rmprefix(char *buf, char ch)
{
	char	*p;

	p = strchr(buf, ch);
	if	( p )	{
		p ++;
		while ( *p == ' ' || *p == '\t' )	p ++;
		strcpy(buf, p);
		}
}

//
void	rmsuffix(char *buf, char ch)
{
	char	*p;

	p = strchr(buf, ch);
	if	( p )	
		*p = '\0';
}

/*
*	true if c is a white-space
*/
int		is_wspace(int c)
{
	return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\b');
}

/*
*	removes spaces and returns a new string
*/
char	*trimdup(const char *str)
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
*/
tnode	*conf_line(int line, tlist *list, const char *text)
{
	if	( strncmp(text, "@re:", 4) == 0 )	{
		const char *error;
		int			errofs;

		tnode *c = (tnode *) list->add(text+4);
		c->type = use_pcre;
		c->re = pcre_compile(text+4, 0, &error, &errofs, NULL);
		die(c->re == NULL, "pcre_compile(), failed (line %d, offset %d)\n-> %s", line, error, errofs);
		return c;
		}
	return (tnode *) list->add(text);
}

/*
*/
void	read_config(const char *file)
{
	FILE	*fp;
	int		mode = 0;
	char	buf[1024], *p;
	int		line = 0;

	fp = fopen(file, "rt");
	if	( fp )	{
		while ( fgets(buf, 1024, fp) )	{
			if	( buf[strlen(buf)-1] == '\n' )	
				buf[strlen(buf)-1] = '\0';
			line ++;
			
			if	( strlen(buf) )	{
				if	( strstr(buf, "@remove_text_lines") )	
					mode = 1;
				else if	( strstr(buf, "@ignore_block_start") )	
					mode = 2;
				else if	( strstr(buf, "@ignore_block_stop") )	
					mode = 3;
				else if	( strstr(buf, "@max_empty_lines") )	
					mode = 4;
				else if	( strstr(buf, "@default_ignore_state") )
					mode = 5;
				else if	( strstr(buf, "@remove_control_chars") )
					mode = 6;
				else if	( strstr(buf, "@redirect") )	
					mode = 7;
				else	{
					switch ( mode )	{
					case	1:
						conf_line(line, &ignline, buf);
						break;
					case	2:
						conf_line(line, &ignstart, buf);
						break;
					case	3:
						conf_line(line, &ignstop, buf);
						break;
					case	4:
						ignlines = atoi(buf);
						break;
					case	5:
						ignore = atoi(buf);
						break;
					case	6:
						ignctrl = atoi(buf);
						break;
					case	7:
						p = strstr(buf, ":");
						if	( p )	{
							*p = '\0';
							tnode *t = conf_line(line, &redir, p+1);
							t->setData(buf);
							}
						else
							die(true, "syntax error");
						break;
					default:
						fprintf(stderr, "I don't know what to do [%s]\n", buf);
						}
					}
				}
			}

		fclose(fp);
		}
	else
		fprintf(stderr, "config not found [%s]\n", file);
}

/*
*/
tnode	*check_list(tlist *list, const char *text)
{
	tnode	*t;
	int		rc;
    int		erroffset;
	int		ovector[OVECCOUNT];

	t = (tnode *) list->head;
	while ( t )	{
		switch ( t->type )	{
		case use_pcre:
			rc = pcre_exec(t->re, NULL, text, strlen(text), 0, 0, ovector, OVECCOUNT);
			if	( rc >= 0 )
				return t;
			break;
		default:
			if	( strstr(text, t->line) )
				return t;
			}

		t = (tnode *) t->next;
		}
	return NULL;
}

/*
*/
int	main(int argc, char *argv[])
{
	char	buf[1024], *trimed;
	int		lnsp = 0, i, idx, c;
	int		no_store = 0, arg_mode = 0;
	char	conf[1024];
	char	file[1024];
	FILE	*fp, *fout;
	tnode	*t;

	// init
	strcpy(conf, "cutof.conf");
	fp = stdin;
	fout = stdout;

	// read args
	for ( i = 1; i < argc; i ++ )	{
		if	( argv[i][0] == '-' )	{
			if	( strcmp(argv[i], "-f") == 0 )
				arg_mode = 1;
			else if	( strcmp(argv[i], "-rc") == 0 )
				ignctrl = 1;
			else if	( strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 )	{
				printf("usage: cutof [-f config-file] [-rc] [input-file]\n");
				printf("-rc = enable cut control characters\n");
				return 1;
				}
			else
				fprintf(stderr, "unknown argument [%s]\n", argv[i]);
			}
		else {
			if	( arg_mode == 1 )	{
				strcpy(conf, argv[i]);
				arg_mode = 0;
				}
			else	{
				strcpy(file, argv[i]);
				fp = fopen(file, "rt");
				if	( !fp )	{
					fprintf(stderr, "cannot open 'input-file' [%s]\n", file);
					return 1;
					}
				}
			}
		}

	read_config(conf);

	// start input
	while ( !feof(fp) )	{
		
		// read text-line
		idx = 0;
		while ( !feof(fp) )	{
			c = fgetc(fp);
			if	( c == 0 )	c = '\r';
			else if	( c == '\n' )
				break;
			if	( c != '\r' )	{
				if	( ignctrl )	{
					if	( c >= 32 )	{
						buf[idx] = c;
						idx ++;
						}
					}
				else	{
					buf[idx] = c;
					idx ++;
					}
				}
			}
		buf[idx] = '\0';
		trimed = trimdup(buf);

		////////////
		
		if	( ignore )	{	// inside ignored block
			if	( check_list(&ignstop, buf) )	{
				ignore = 0;
				lnsp = 0;
				}
			}
		else	{	// inside not-ignored block
			// check start-ignore block
			if	( check_list(&ignstart, buf) )	
				ignore = 1;

			//
			no_store = 0;

			// check ignore text-line
			if	( check_list(&ignline, buf) )	
				no_store = 1;

			// check empty lines
			if	( strlen(trimed) == 0 )	{
				lnsp ++;
				if	( lnsp > ignlines )
					no_store = 1;
				}
			else {
				if	( no_store == 0 )
					lnsp = 0;
				}

			// check for redirection
			if	( (t = check_list(&redir, buf)) != NULL )	{
				if	( fout != stdout )
					fclose(fout);
				fout = fopen(t->data, "wt");
				if	( !fout )	{
					fprintf(stderr, "cannot redir [%s]\n", t->data);
					fout = stdout;
					}
				}
			}

		// print
		if	( no_store == 0 && ignore == 0 )	
			fprintf(fout, "%s\n", buf);

		free(trimed);
		}

	return 0;
}

