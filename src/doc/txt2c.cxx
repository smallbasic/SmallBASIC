/*
*	guide.txt -> C arrays for on-line help
*
*	Nicholas Christopoulos
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

// keywords
static char *kFunc = "_F_u_n_c_t_i_o_n_: ";
static char *kProc = "_C_o_m_m_a_n_d_: ";
static char *kStmn = "_S_t_a_t_e_m_e_n_t_: ";
static char *kMacr = "_M_a_c_r_o_: ";

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

	tnode(const char *src) : node()	{ line = pstrdup(src);	}
	virtual ~tnode()				{ delete[] line;		}
	};

// list / content type
enum doc_t	{ doc_unknown, doc_section, doc_macro, doc_stmn, doc_proc, doc_func };

//
const char *doc2str(doc_t type)
{
	const char *p;

	switch ( type )	{
	case doc_section:	p = "SECTION";		break;
	case doc_macro:		p = "MACRO";		break;
	case doc_stmn:		p = "STATEMENT";	break;
	case doc_proc:		p = "COMMAND";		break;
	case doc_func:		p = "FUNCTION";		break;
	default:
		p = "err";
		}

	return p;
}

// text list
class tlist : public list {
public:
	char	*title;			// just a text
	char	*key;			// -//-
	char	*syntax;		// -//-
	doc_t	type;
	int		level;

	tlist() : list()	{ title = key = syntax = NULL; level = 0; }
	virtual ~tlist()	{ 
		if ( title  ) delete[] title; 
		if ( key    ) delete[] key; 
		if ( syntax ) delete[] syntax; 
		}

	// set title
	void	setTitle(const char *text)	{ 
		if ( title ) delete[] title;
		title = pstrdup(text);	
		}

	// set key
	void	setKey(const char *text)	{ 
		if ( key ) delete[] key;
		key = pstrdup(text);	
		}

	// set syntax
	void	setStx(const char *text)	{ 
		if ( syntax ) delete[] syntax;
		syntax = pstrdup(text);
		}

	// add a new line
	void	add(const char *text)	{
		tnode	*np = new tnode(text);
		connect(np);
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

	// add a new named buffer
	tlist	*add(const char *title)	{
		tlist	*np = add();
		np->setTitle(title);
		return np;
		}
	};

///////////////////////////////////////////////////////////////////////////////////////////////////////////

buflist	*buffers;

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

// section level, 0 = none
int		seclevel(const char *src)
{
	int		l = 1;
	const char *p;

	p = src;
	while ( *p )	{
		if ( *p == '.' && *(p+1) != ' ' )
			l ++;
		if	( *p == ' ')
			break;
		p ++;
		}

	return l;
}

//
char	hexdig1(char c)
{
	int		x = c >> 4;

	return "0123456789ABCDEF"[x];
}

//
char	hexdig2(char c)
{
	int		x = c & 0xF;

	return "0123456789ABCDEF"[x];
}

// convert string to C style
void	ccstr(const char *src, char *dst)
{
	const char	*p = src;
	char		*d = dst;

	while ( *p )	{
		if	( *p == '\\' )	
			( *d ++ = *p, *d ++ = *p );
		else if	( *p == '\"' )	
			( *d ++ = '\\', *d ++ = *p );
		else if	( *p == '\'' )	
			( *d ++ = '\\', *d ++ = *p );
		else if	( *p == 8 )	
			( *d ++ = '\\', *d ++ = 'b' );
		else if	( *p == 9 )	
			( *d ++ = '\\', *d ++ = 't' );
		else if	( *p < 32 && *p > 0 )	{	
			( *d ++ = '\\', *d ++ = 'x', *d ++ = hexdig1(*p), *d ++ = hexdig2(*p) );
			}
		else
			*d ++ = *p;

		p ++;
		}

	*d = '\0';
}

//
void	setkey(const char *text, int level, char *buf)
{
	char	tmp[4096], *p;

	if	( level )	{
		if	( level == 1 )
			sprintf(buf, "%s", text);
		else if ( level == 2 )	{
			p = strchr(buf, '/');
			if	( p )	
				*p = '\0';

			strcpy(tmp, buf);
			sprintf(buf, "%s/%s", tmp, text);
			}
		else if ( level == 3 )	{
			p = strchr(buf, '/');
			if	( p )	{
				p = strchr(p+1, '/');
				if	( p )
					*p = '\0';
				}

			strcpy(tmp, buf);
			sprintf(buf, "%s/%s", tmp, text);
			}
		else if ( level == 4 )	{
			p = strchr(buf, '/');
			if	( p )	{
				p = strchr(p+1, '/');
				if	( p )	{
					p = strchr(p+1, '/');
					if	( p )	
						*p = '\0';
					}
				}

			strcpy(tmp, buf);
			sprintf(buf, "%s/%s", tmp, text);
			}
		}
}

//
static bool opt_debug = false;

//
int main(int argc, char *argv[])
{
	char	line[1024];
	char	buf[1024];
	char	key[4096];
	FILE	*fi, *fo;
	int		i, l;

	// init
	buffers = new buflist;				// list of buffers
	tlist	*clist = buffers->add();	// current buffer
	tlist	*glist = clist;				// garbage 
	fi = stdin;							// input file
	fo = stdout;						// output file
	strcpy(key, "");

	// check arguments
	for ( i = 1; i < argc; i ++ )	{
		if	( argv[i][0] == '-' )	{
			switch ( argv[i][1] )	{
			case 'h':
				printf("usage: txt2c [-d] [input] [output]\n");
				return 1;
			case 'd':
				opt_debug = true;
				break;
			default:
				die(true, "unknown argument '%s'", argv[i]);
				}
			}
		else	{
			// filename
			strcpy(buf, argv[i]);
			if	( fi == stdin )
				die((fi = fopen(buf, "rt")) == NULL, "can't open input file '%s'", buf);
			else if	( fo == stdout )
				die((fo = fopen(buf, "wt")) == NULL, "can't create output file '%s'", buf);
			else
				die(true, "unknown argument '%s'", argv[i]);
			}
		}

	// process file
	while ( fgets(line, 1024, fi) )	{
		if	( line[strlen(line)-1] == '\n' )
			line[strlen(line)-1] = '\0';

		if	( strchr(line, '\b') )	{
			// it is keyword
			if	( strstr(line, kStmn) )	{
				// statement declaration
				cleantext(line, buf);
				rmprefix(buf, ':');
				rmsuffix(buf, ' ');
				clist = buffers->add(buf);
				clist->type = doc_stmn;
				clist->setKey(key);
				rmprefix(line, ':');
				clist->setStx(line);
				}
			else if	( strstr(line, kProc) )	{
				// command declaration
				cleantext(line, buf);
				rmprefix(buf, ':');
				rmsuffix(buf, ' ');
				clist = buffers->add(buf);
				clist->type = doc_proc;
				clist->setKey(key);
				rmprefix(line, ':');
				clist->setStx(line);
				}
			else if	( strstr(line, kFunc) )	{
				// function declaration
				cleantext(line, buf);
				rmprefix(buf, ':');
				rmsuffix(buf, ' ');
				clist = buffers->add(buf);
				clist->type = doc_func;
				clist->setKey(key);
				rmprefix(line, ':');
				clist->setStx(line);
				}
			else if	( strstr(line, kMacr) )	{
				// macro declaration
				cleantext(line, buf);
				rmprefix(buf, ':');
				rmsuffix(buf, ' ');
				clist = buffers->add(buf);
				clist->type = doc_macro;
				clist->setKey(key);
				rmprefix(line, ':');
				clist->setStx(line);
				}
			else if ( 
					(isdigit(line[0]) && line[1] == '.' && line[2] == ' ' ) ||	// chapter/section...
					(isdigit(line[0]) && line[1] == '.' && isdigit(line[2]) ) ||	// chapter/section...
					(isdigit(line[0]) && isdigit(line[1]) && line[2] == '.' && line[3] == ' ' ) ||	// chapter/section...
					(isalpha(line[0]) && line[1] == '.' && line[2] == ' ')	// appendix...
					)	{
				// text
				cleantext(line, buf);
				l = seclevel(buf);
				rmprefix(buf, ' ');
				clist = buffers->add(buf);
				clist->type = doc_section;
				clist->level = l;
				setkey(buf, l, key);
				clist->setKey(key);
				}
			else {
				// if non-care element:	clist = glist;
				// default add: clist->add(line);

				// warning: lynx does not using \b for H
				clist->add(line);
				}
			}
		else
			clist->add(line);
		}
	
	// print results
	if	( opt_debug )	{
		tlist	*cl = (tlist *) buffers->head;
		while ( cl )	{
			tnode	*ct = (tnode *) cl->head;
			while ( ct )	{
//				printf("(%s:%d:[%s]) %s: %s\n", doc2str(cl->type), cl->level, cl->key, cl->title, ct->line);
				printf("(%s:%d) %s: %s\n", doc2str(cl->type), cl->level, cl->title, ct->line);
				ct = (tnode *) ct->next;	// next text line
				}

			cl = (tlist *) cl->next;	// next buffer
			}
		}
	else	{
		tlist	*cl;
		tnode	*ct;
		int		i;

		fprintf(fo, "/* txt2c - SB CLI Help */\n");
		fprintf(fo, "/* structure */\n");
		fprintf(fo, "typedef struct { const char *code; const char *type; const char *syntax; const char *descr; } help_node_t;\n\n");
		fprintf(fo, "help_node_t help_data[] = {\n");

		for ( i = doc_macro; i <= doc_func; i ++ )	{
			// for each buffer
			cl = (tlist *) buffers->head;
			while ( cl )	{
				if	( cl->type == i )	{
					ct = (tnode *) cl->head;
					fprintf(fo, "{ \"%s\", \"%s\", \n", cl->title, doc2str(cl->type));
					strcpy(buf, cl->syntax);
					ccstr(buf, line);
					fprintf(fo, "\"%s\", \n", line);

					// print descr
					while ( ct )	{
						strcpy(buf, ct->line);
						ccstr(buf, line);
						fprintf(fo, "\"%s\\n\"\n", line);
						ct = (tnode *) ct->next;	// next text line
						}

					// close
					fprintf(fo, "},\n\n");
					}
				cl = (tlist *) cl->next;	// next buffer
				}
			}

		fprintf(fo, "{ NULL, NULL, NULL, NULL } };\n");
		}

	// cleanup
	delete buffers;
}
