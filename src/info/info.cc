/**
*	Info, man-page and text-file class
*
*	2002-10-12, Nicholas Christopoulos
*
*	Linux, xterm and ANSI terminals are supported
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*/

#include "info.h"

// --- INFO-LINE ------------------------------------------------------
Info::InfoLine::InfoLine()
{
	head = tail = NULL;
	text = NULL;
	next = NULL;
	parent = NULL;
}

// clear list
void	Info::InfoLine::clear()
{
	Info::InfoElem	*cur, *pre;

	cur = head;
	while ( cur )	{
		pre = cur;
		cur = cur->next;
		delete pre;
		}
	head = tail = NULL;
}

//
Info::InfoLine::~InfoLine()
{
	clear();
	if	( text )
		delete[] text;
}

//
void	Info::InfoLine::addElem(Info::info_elem_t tp, const char *tpos, const char *key)
{
	Info::InfoElem	*enew = new Info::InfoElem;

	enew->type = tp;
	enew->pos  = tpos;
	enew->len  = strlen(key);
	
	if	( !head )
		( head = tail = enew );
	else
		( tail->next = enew, tail = enew );
}

// --- INFO-NODE ------------------------------------------------------

// constructor: create an info-node
Info::InfoNode::InfoNode()
{
	head = tail = NULL;
	name = strNext = strPrev = strUp = infoFile = NULL;
	chpNext = chpPrev = chpUp = NULL;
	next = NULL;
}

// clear text-nodes
void	Info::InfoNode::clear()
{
	Info::InfoLine	*cur, *pre;

	cur = head;
	while ( cur )	{
		pre = cur;
		cur = cur->next;
		delete pre;
		}
	head = tail = NULL;
}

// destructor
Info::InfoNode::~InfoNode()
{
	clear();

	if	( name )
		delete[] name;
	if	( infoFile )
		delete[] infoFile;
	if	( strNext )
		delete[] strNext;
	if	( strPrev )
		delete[] strPrev;
	if	( strUp )
		delete[] strUp;
}

// add a text line
void	Info::InfoNode::addText(const char *source)
{
	Info::InfoLine	*line = new Info::InfoLine;

	line->parent = (void *) this;

	line->text = new char[strlen(source)+1];
	strcpy(line->text, source);

	if	( !head )
		( head = tail = line );
	else
		( tail->next = line, tail = line );

	// scan elements
	char	*p, *ps;
	char	key[1024], *kp;
	bool	quotes = false;

	p = line->text;
	while ( *p )	{
		if	( *p == '\"' )	{
			quotes = !quotes;
			p ++;
			}
		else if ( !quotes )	{
			if	( *p == '*' )	{
				// menu or note... anyway anything that starts with '*'
				ps = p;
				kp = key;
				while ( *p )	{
					if	( *p == ':' )
						break;
					*kp ++ = *p ++;
					}
				*kp = '\0';

				//
				if	( *p == ':' )	{
					*p = ' ';
					if	( *(p+1) == ':' )
						*(p+1) = ' ';

					if	( strncasecmp(key, "* Menu", 6) == 0 )
						strcpy(ps, "Menu:");
					else if	( strncasecmp(key, "*Note ", 6) == 0 )
						line->addNote(ps+4, key+4);
					else
						line->addMenu(ps, key);
					}
				}
			else if	( *p == '`' && strchr(p, '\'') )	{
				// text between ` and '
				ps = p+1;	p ++;
				kp = key;
				while ( *p )	{
					if	( *p == '\'' )
						break;
					*kp ++ = *p ++;
					}
				*kp = '\0';

				//
				if	( strlen(ps) )	
					line->addItalics(ps, key);
				}
			else
				p ++;
			}
		else
			p ++;
		}
}

// add a text line
void	Info::InfoNode::addSimpleText(const char *source)
{
	Info::InfoLine	*line = new Info::InfoLine;

	line->parent = (void *) this;

	line->text = new char[strlen(source)+4];
	strcpy(line->text, source);

	if	( !head )
		( head = tail = line );
	else
		( tail->next = line, tail = line );

	// scan elements
	char	*p, *ps;
	char	key[1024], *kp;
	char	tex[1024], *tp;
	int		mode = 0;

	p = strchr(line->text, '\b');
	if	( p )	{
		
		// p = the source
		memset(tex, 0, 1024);
		strcpy(tex, source);
		p  = tex;

		// tp = the line
		memset(line->text, 0, strlen(source)+4);
		tp = line->text;

		while ( *p )	{
			if	( *p == '_' && *(p+1) == '\b' )	{
				// italics
				mode = 1;
				}
			else if ( *(p+1) == '\b' ) // bold
				mode = 2;

			if	( mode )	{
				ps = tp;
				kp = key;
				while ( *p )	{
					if	( strchr("[]()&| \t;{}+=&^$#@!\n\r", *p) && *(p+1) != '\b' )
						break;

					if	( *p == '\b' )	
						( kp --, tp --, p ++ );
					else	
						( *kp ++ = *p, *tp ++ = *p ++ );
					}
				*kp = '\0';

//				fprintf(stderr, "[%s] [%s]\n", ps, key);
				if	( mode == 2 )
					line->addItalics(ps, key);
				else
					line->addBold(ps, key);

				mode = 0;
				}

			if	( *p == '\b' )
				( tp --, p ++ );
			else
				*tp ++ = *p ++;
			}

		*tp = '\0';
		}
}

// --- INFO -----------------------------------------------------------

// default constructor
Info::Info()
{
	head = tail = NULL;
}

// clear list
void	Info::clear()
{
	InfoNode	*cur, *pre;

	cur = head;
	while ( cur )	{
		pre = cur;
		cur = cur->next;
		delete pre;
		}
	head = tail = NULL;
}

// destructor
Info::~Info()
{
	clear();
}

// returns the InfoLine node which contains the string 'str'
Info::InfoLine*	Info::findString(const char *str, Info::InfoLine *start)
{
	Info::InfoNode	*cur;
	Info::InfoLine	*line;

	if	( !str )			return NULL;
	if	( !strlen(str) )	return NULL;

	//
	char	*p = (char *) str;
	while ( *p == ' ' || *p == '\t' )	p ++;
	if	( *p == '\0' )		return NULL;		// empty string

	// create a "trimmed" version of looking string
	char	*lookFor = new char[strlen(p)+1];
	strcpy(lookFor, p);
	p = lookFor + (strlen(lookFor) - 1);
	while ( *p == ' ' || *p == '\t' )	p --;
	*(p+1) = '\0';

	// search on text lines
	if	( start )	
		cur = (Info::InfoNode *) start->parent;
	else
		cur = head;

	while ( cur )	{
		if	( start )	{
			if	( cur == (Info::InfoNode *) start->parent )	
				line = start->next;
			else
				line = cur->head;
			}
		else
			line = cur->head;

		while ( line )	{
			if	( strcasecmp(lookFor, line->text) == 0 )	{
				delete[] lookFor;
				return line;
				}
			line = line->next;
			}
		cur = cur->next;
		}
	
	// not found
	delete[] lookFor;
	return NULL;
}

/*
*	search a set of directories for the given file
*	directories on path must be separated with symbol ':' (unix) or ';' (dos/win)
*
*	@param path the path
*	@param file the file
*	@param retbuf a buffer to store the full-path-name file (can be NULL)
*	@return non-zero if found
*/
int		Info::sys_search_path(const char *path, const char *file, char *retbuf)
{
	const char	*ps, *p;
	char	cur_path[1024];

	if	( path == NULL )
		return 0;
	if	( strlen(path) == 0 )
		return 0;

	ps = path;
	do	{
		// next element, build cur_path
		#if defined(_UnixOS)
		p = strchr(ps, ':');
		#else
		p = strchr(ps, ';');
		#endif
		if	( !p )	
			strcpy(cur_path, ps);
		else	{
			strncpy(cur_path, ps, p-ps);
			cur_path[p-ps] = '\0';
			ps = p+1;
			}

		// fix home directory
		if	( cur_path[0] == '~' )	{
			char	*old_path;

			old_path = new char[strlen(cur_path)];
			strcpy(old_path, cur_path+1);
			#if defined(_UnixOS)
			sprintf(cur_path, "%s/%s", getenv("HOME"), old_path);
			#else
			if	( getenv("HOME") )	
				sprintf(cur_path, "%s\\%s", getenv("HOME"), old_path);
			else
				sprintf(cur_path, "%s\\%s", getenv("HOMEPATH"), old_path);
			#endif
			delete[] old_path;
			} 

		// build the final file-name
		#if defined(_UnixOS)
		strcat(cur_path, "/");
		#else
		strcat(cur_path, "\\");
		#endif
		strcat(cur_path, file);

		// TODO: probably in DOS/Win we must remove double dir-seps (c:\\home\\\\user)

		// check it
//		printf("sp:[%s]\n", cur_path);
		if	( access(cur_path, R_OK) == 0 )	{
			if	( retbuf )
				strcpy(retbuf, cur_path);
			return 1;
			}

		} while ( p );

	return 0;
}

//
Info::InfoNode*	Info::addNode()
{
	Info::InfoNode	*node = new Info::InfoNode;

	if	( !head )
		( head = tail = node );
	else
		( tail->next = node, tail = node );
}

//
Info::InfoNode*	Info::findNode(const char *name)
{
	Info::InfoNode	*cur;
	Info::InfoLine	*line;
	Info::InfoElem	*elem;

	if	( !name )			return NULL;
	if	( !strlen(name) )	return NULL;

	//
	char	*p = (char *) name;
	while ( *p == ' ' || *p == '\t' )	p ++;
	if	( *p == '\0' )		return NULL;		// empty string

	// create a "trimmed" version of looking string
	char	*lookFor = new char[strlen(p)+1];
	strcpy(lookFor, p);
	p = lookFor + (strlen(lookFor) - 1);
	while ( *p == ' ' || *p == '\t' )	p --;
	*(p+1) = '\0';

	// search node's titles
	cur = head;
	while ( cur )	{
		if	( cur->name )	{
			if	( strcasecmp(lookFor, cur->name) == 0 )	{
				delete[] lookFor;
				return cur;
				}
			}
		cur = cur->next;
		}

	// search section & subsection titles
	cur = head;
	while ( cur )	{
		line = cur->head;
		while ( line )	{
			elem = line->head;
			while ( elem )	{
				if	( elem->type == ie_section || elem->type == ie_subsection )	{
					if	( strcasecmp(lookFor, line->text) == 0 )	{
						delete[] lookFor;
						return cur;
						}
					}
				elem = elem->next;
				}
			line = line->next;
			}
		cur = cur->next;
		}
	
	delete[] lookFor;
	return NULL;
}

//
int		Info::sparse(const char *source, const char *fmt, ...)
{
	const char	*fmt_p = fmt;
	const char	*p = source;
	va_list		ap;
	int			pcount = 0;
	char		key[1024], *kp;

	va_start(ap, fmt);

	// parse
	while ( *fmt_p )	{
		// ignore spaces (fmt)
		if ( *fmt_p == ' ' || *fmt_p == '\t' )	{
			fmt_p ++;
			continue;
			}

		// get keyword
		kp = key;
		while ( *fmt_p && *fmt_p != '%' )
			*kp ++ = *fmt_p ++;
		*kp = '\0';

		// parameter
		if ( *fmt_p == '%' ) {
			fmt_p ++;

			if	( *fmt_p == 's'  )	{
				// string
				char	**s;
				const char *ps;
				char	end, *tp;
				bool	quotes = false;

				s = va_arg(ap, char **);
				*s = NULL;

				fmt_p ++;
				end = *fmt_p;
				fmt_p ++;

				p = strstr(source, key);

				if	( p )	{
					p += strlen(key);
					while ( *p == ' ' || *p == '\t' )	p ++;

					pcount ++;

					// calc string
					ps = p;
					while ( *p )	{
						if	( *p == '\"' )
							quotes = !quotes;
						else if ( !quotes )	{
							if	( *p == '\n' )	break;
							if	( *p == end )	break;
							}

						p ++;
						}
					// copy string
					*s = new char[(p-ps)+1];
					tp = *s;
					strncpy(tp, ps, (p-ps));
					tp[p-ps] = '\0';

					// remove quotes
					if	( (tp = strchr(*s, '\"')) )	{
						char	*ts = tp+1;
						char	*ss = *s;

						while ( *ts && *ts != '\"' )
							*ss ++ = *ts ++;
						*ss = '\0';
						}
					}	// *p
				} // == 's'

			//
			continue;
			} // parameter
		}

	va_end(ap);
	return pcount;
}

//
bool	Info::loadInfoPart(const char *file)
{
	FILE		*fp;
	char		buf[1024], *p;	
	InfoNode	*node = NULL;
	int			mode = 0;

	fp = fopen(file, "rb");
	if	( fp )	{
		while ( fgets(buf, 1024, fp) )	{
			buf[strlen(buf)-1] = '\0';

			if	( buf[0] == 0x1F )	{
				node = addNode();	// create new node
				mode = 1;			// next line had info
				}
			else if ( mode == 1 )	{
				if	( strncasecmp(buf, "File:", 5) == 0 )	{
					char	*f, *n, *v, *x, *u;

					sparse(buf, "File:%s,Node:%s,Prev:%s,Next:%s,Up:%s", &f, &n, &v, &x, &u);

					node->name = n;
					node->infoFile = f;
					node->strNext = x;
					node->strPrev = v;
					node->strUp = u;

					mode = 2;		// add to node
					}
				else if	( strncasecmp(buf, "Indirect:", 9) == 0 )	{
					mode = 3;		// load file
					}
				else
					mode = 0;		// wait for 0x1F
				}
			else if ( mode == 2 )	{
				// is is of section title
				p = buf;
				while ( *p == '=' )	p ++;
				if	( *p == '\0' )	{
					if	( node->tail )	{
						if	( node->tail->text ) {
							if ( strlen(buf) == strlen(node->tail->text) )	{
								node->tail->addSectionTitle();
								}
							} // text
						} // tail
					}

				// is is of sub-section title
				p = buf;
				while ( *p == '*' )	p ++;
				if	( *p == '\0' )	{
					if	( node->tail )	{
						if	( node->tail->text ) {
							if ( strlen(buf) == strlen(node->tail->text) )	{
								node->tail->addSubSectionTitle();
								}
							} // text
						} // tail
					}

				// simple text
				node->addText(buf);
				}
			else if ( mode == 3 )	{	// load file
				p = strchr(buf, ':');
				if	( p )
					*p = '\0';
				p = buf;
				while ( *p == ' ' || *p == '\t' )	p ++;
				if	( strlen(p) )	{
					if	( !loadInfo(p) )	
						fprintf(stderr, "%s: not found\n", p);
					}
				}
			}
		fclose(fp);

		// connect nodes
		Info::InfoNode	*cur;

		cur = head;
		while ( cur )	{
			cur->chpUp   = findNode(cur->strUp);
			cur->chpPrev = findNode(cur->strPrev);
			cur->chpNext = findNode(cur->strNext);

			cur = cur->next;
			}
		}
	else
		return false;

	return true;
}

//
bool	Info::loadText(const char *file)
{
	FILE		*fp;
	char		buf[1024], *p;	
	InfoNode	*node = NULL;
	int			mode = 0;

	fp = fopen(file, "rb");
	if	( fp )	{
		node = addNode();	// create new node
		node->name = new char[4];
		strcpy(node->name, "Top");
		node->infoFile = new char[strlen(file)+1];
		strcpy(node->infoFile, file);

		while ( fgets(buf, 1024, fp) )	{
			buf[strlen(buf)-1] = '\0';
			node->addSimpleText(buf);
			}
		fclose(fp);
		}
	else
		return false;

	return true;
}

//
bool	Info::loadInfo(const char *file)
{
	char	full[1024];	
	char	buf[1024];	
	char	tmp[1024];	
	char	*def_path =
			"./"
			"/usr/info:"
			"/usr/share/info:"
			"/usr/local/info:";

	FILE		*fp;
	InfoNode	*node = NULL;
	int			mode = 0;
	bool		rv = false;

	//groff -Tascii -s -p -t -e -mandoc /usr/man/???.?

	// .info.gz
	sprintf(full, "%s.info.gz", file);
	if	( sys_search_path(def_path, full, full) )	{
		sprintf(tmp, "/tmp/%s-%d.info", file, getpid());
		sprintf(buf, "gzip -d -q -c %s > %s", full, tmp);
		system(buf);
		strcpy(full, tmp);
		rv = loadInfoPart(full);
		remove(full);
		return rv;
		}

	// .info.Z
	sprintf(full, "%s.info.Z", file);
	if	( sys_search_path(def_path, full, full) )	{
		sprintf(tmp, "/tmp/%s-%d.info", file, getpid());
		sprintf(buf, "gzip -d -q -c %s > %s", full, tmp);
		system(buf);
		strcpy(full, tmp);
		rv = loadInfoPart(full);
		remove(full);
		return rv;
		}

	// .info.bz2
	sprintf(full, "%s.info.bz2", file);
	if	( sys_search_path(def_path, full, full) )	{
		sprintf(tmp, "/tmp/%s-%d.info", file, getpid());
		sprintf(buf, "bzip2 -d -c %s > %s", full, tmp);
		system(buf);
		strcpy(full, tmp);
		rv = loadInfoPart(full);
		remove(full);
		return rv;
		}

	// .info
	sprintf(full, "%s.info", file);
	if	( sys_search_path(def_path, full, full) )	{
		rv = loadInfoPart(full);
		return rv;
		}

	// .gz
	sprintf(full, "%s.gz", file);
	if	( sys_search_path(def_path, full, full) )	{
		sprintf(tmp, "/tmp/%s-%d.info", file, getpid());
		sprintf(buf, "gzip -d -q -c %s > %s", full, tmp);
		system(buf);
		strcpy(full, tmp);
		rv = loadInfoPart(full);
		remove(full);
		return rv;
		}

	// .Z
	sprintf(full, "%s.Z", file);
	if	( sys_search_path(def_path, full, full) )	{
		sprintf(tmp, "/tmp/%s-%d.info", file, getpid());
		sprintf(buf, "gzip -d -q -c %s > %s", full, tmp);
		system(buf);
		strcpy(full, tmp);
		rv = loadInfoPart(full);
		remove(full);
		return rv;
		}

	// .bz2
	sprintf(full, "%s.bz2", file);
	if	( sys_search_path(def_path, full, full) )	{
		sprintf(tmp, "/tmp/%s-%d.info", file, getpid());
		sprintf(buf, "bzip2 -d -c %s > %s", full, tmp);
		system(buf);
		strcpy(full, tmp);
		rv = loadInfoPart(full);
		remove(full);
		return rv;
		}

	// .
	sprintf(full, "%s", file);
	if	( sys_search_path(def_path, full, full) )	{
		rv = loadInfoPart(full);
		return rv;
		}

	clear();	// failed
	return rv;
}

//
bool	Info::loadMan(const char *file)
{
	char	full[1024];	
	char	buf[1024];	
	char	tmp[1024];	
	char	*def_path =
			"./"
			;

	FILE		*fp;
	InfoNode	*node = NULL;
	int			i;
	bool		rv = false;
	char		manLet[] = "123456789nl";

	// .?.gz
	for ( i = 1; i < strlen(manLet); i ++ )	{
		sprintf(full, "%s.%c", file, manLet[i]);
		if	( sys_search_path(def_path, full, full) )	{

			sprintf(tmp, "/tmp/%s-%d.man", file, getpid());
			sprintf(buf, "groff -Tascii -s -p -t -e -mandoc %s > %s", full, tmp);
			system(buf);

			strcpy(full, tmp);
			rv = loadText(full);
			remove(full);
			return rv;
			}
		}

	// all man
	sprintf(tmp, "/tmp/%s-%d.man", file, getpid());
	sprintf(buf, "man %s > %s", file, tmp);
	system(buf);

	strcpy(full, tmp);
	rv = loadText(full);
	remove(full);
	return rv;
}

