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

#if !defined(__ndc_info_h)
#define __ndc_info_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>

// info-file reader
class Info {
public:
	// type of element
	enum info_elem_t	{
		ie_menu,		// menu item
		ie_note,		// note
		ie_italics,		// `..'
		ie_bold,		// ....
		ie_section,		// 
		ie_subsection,	//
		ie_nil			// nothing
		};

	// element
	class InfoElem	{
	public:
		info_elem_t	type;		// type
		const char	*pos;		// position in text
		int			len;		// length of text

		InfoElem	*next;		// list's next ptr

		InfoElem()		{ pos = NULL; len = 0; type = ie_nil; next = NULL; }
		virtual ~InfoElem()	{ }
		};

	// text line
	class InfoLine	{
	public:
		char		*text;			// the text
		InfoElem	*head, *tail;	// element's list

		InfoLine	*next;			// list's next ptr

		void		*parent;		// Info::InfoNode ptr

		InfoLine();
		virtual ~InfoLine();

		void	clear();			// clear the element's list
		void	addElem(Info::info_elem_t tp, const char *tpos, const char *key);
		void	addNote(const char *tpos, const char *key)
					{ addElem(ie_note, tpos, key); }
		void	addMenu(const char *tpos, const char *key)
					{ addElem(ie_menu, tpos, key); }
		void	addItalics(const char *tpos, const char *key)
					{ addElem(ie_italics, tpos, key); }
		void	addBold(const char *tpos, const char *key)
					{ addElem(ie_bold, tpos, key); }
		void	addSectionTitle()
					{ addElem(ie_section, text, text); }
		void	addSubSectionTitle()
					{ addElem(ie_subsection, text, text); }
		};

	// info-node
	class InfoNode {
	public:
		char		*name;			// Node's title
		char		*strNext;		// Next-node title (or NULL)
		char		*strPrev;		// Prev-node title (or NULL)
		char		*strUp;			// Up-node title (or NULL)
		char		*infoFile;		// Info's file-name

	public:		// do not use them
		InfoLine	*head, *tail;	// list of info-lines
		InfoNode	*next;			// list's next ptr
		
		InfoNode	*chpNext;		// chapter: next ptr
		InfoNode	*chpPrev;		// chapter: previous ptr
		InfoNode	*chpUp;			// chapter: up ptr

	public:
		InfoNode();
		virtual ~InfoNode();

		void	addText(const char *text);
		void	addSimpleText(const char *text);

		void	clear();			// clear info-line list
		};

private:
	Info::InfoNode	*head, *tail;					// the node's list

	Info::InfoNode*	addNode();					// create node
	bool	loadInfoPart(const char *file);		// internal: loading info

	int		sys_search_path(const char *path, const char *file, char *retbuf);	// search path

	int		sparse(const char *source, const char *fmt, ...);					// parse strings

public:
	Info();
	virtual ~Info();

	void	clear();		  					// clear all

	Info::InfoNode*	findNode(const char *name);			// use "Top" to find the first node (or the whole text of man/text-files)
	
	// returns the InfoLine node which contains the string 'str'
	Info::InfoLine*	findString(const char *str, Info::InfoLine *start = NULL);

	bool	loadInfo(const char *file);			// load .info file
	bool	loadText(const char *file);			// load text file
	bool	loadMan(const char *file);			// load .[1-9] groff (man) file
	};

#endif

