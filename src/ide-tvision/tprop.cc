/**
*	Properties class
*
*	Similar to .INI files
*
*	There are 3 modes to work with this class
*
*	a) on file					(!local && !memoryFile)
*	b) read once, write once 	( local && !memoryFile)
*	c) on memory				( local	&&  memoryFile)
*
*	Nicholas Christopoulos
*/

#include <tv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tprop.hpp"

///////////////////////////////////////////////////////////
// TProp::TSection::TVar  /////////////////////////////////

TProp::TSection::TVar::TVar(const char *varName, const char *varValue)
{
	name = new char[strlen(varName)+1];
	strcpy(name, varName);

	if	( varValue )	{
		value = new char[strlen(varValue)+1];
		strcpy(value, varValue);
		}
	else	{
		value = new char[1];
		strcpy(value, "");
		}
}

TProp::TSection::TVar::~TVar()
{
	delete[] name; 
	delete[] value; 
}

/**
*	setup variable's value (string)
*
*	@param val the text
*/
void	TProp::TSection::TVar::setValue(const char *val)
{
	delete[] value;

	if	( val )	{
		value = new char[strlen(val)+1];
		strcpy(value, val);
		}
	else	{
		value = new char[1];
		strcpy(value, "");
		}
}

/**
*	setup variable's value (integer)
*
*	@param val the integer
*/
void	TProp::TSection::TVar::setValue(int val)
{
	char	buf[32];

	sprintf(buf, "%d", val);
	setValue(buf);
}

/**
*	setup variable's value (double)
*
*	@param val the double
*/
void	TProp::TSection::TVar::setValue(double val)
{
	char	buf[64];

	sprintf(buf, "%f", val);
	setValue(buf);
}

///////////////////////////////////////////////////////////
// TProp::TSection::TNode /////////////////////////////////

TProp::TSection::TNode::TNode(fileNodeType_t ntype, const char *varName, const char *varValue)
{
	type = ntype;
	if	( type == fnt_var )	{
		var = new TVar(varName, varValue);
		text = NULL;
		}
	else	{
		text = new char[strlen(varName)+1];
		strcpy(text, varName);
		var = NULL;
		}

	next = NULL;
}

TProp::TSection::TNode::~TNode()
{
	if	( type == fnt_var )
		delete var;
	else
		delete[] text;
}

///////////////////////////////////////////////////////////
// TProp::TSection        /////////////////////////////////

TProp::TSection::TSection(const char *secName)
{ 
	head = tail = NULL; 
	next = NULL; 
	name = new char[strlen(secName)+1]; 
	strcpy(name, secName);
}

TProp::TSection::~TSection()				
{
	clear(); 
	delete[] name;
}

/**
*	clears the TNode list
*/
void	TProp::TSection::clear()
{
	TProp::TSection::TNode	*curNode, *prevNode;

	curNode = head;
	while ( curNode )	{
		prevNode = curNode;
		curNode = curNode->getNextPtr();
		delete prevNode;
		}

	head = tail = NULL;
}

/**
*	returns the variable-object which had the specified name
*
*	@param name is the variable-name
*	@return if found the variable-object; otherwise returns NULL
*/
TProp::TSection::TVar* TProp::TSection::findVar(const char *name) const
{
	TProp::TSection::TNode	*node;
	TProp::TSection::TVar	*var;

	node = head;
	while ( node )	{
		if	( node->isVar() )	{
			var = node->getVarPtr();
			if	( strcmp(var->getName(), name) == 0 )
				return var;	// found
			}
		node = node->getNextPtr();
		}
	return NULL;
}

/**
*	adds a new variable
*
*	@param name is the variable-name
*	@param val  is the value
*	@return the new variable-object
*/
TProp::TSection::TVar* TProp::TSection::addVar(const char *name, const char *val)
{
	TProp::TSection::TNode	*node;

	node = new TProp::TSection::TNode(fnt_var, name, val);

	if	( !head )
		head = tail = node;
	else	{
		tail->setNextPtr(node);
		tail = node; 
		}
	return node->getVarPtr();
}

/**
*	adds a new comments-line
*
*	@param text is the text
*	@return the new TNode
*/
TProp::TSection::TNode* TProp::TSection::addText(const char *text)
{
	TProp::TSection::TNode	*node;

	node = new TProp::TSection::TNode(fnt_text, text);

	if	( !head )
		head = tail = node;
	else	{
		tail->setNextPtr(node);
		tail = node;
		} 
	return node;
}

///////////////////////////////////////////////////////////
// TProp                  /////////////////////////////////

/**
*	constructor
*
*	@param file the .ini filename
*/
TProp::TProp(const char *file, bool blocal)
{
	local = blocal;
	loaded = false;
	memoryFile = false;

	strcpy(fileName, file);
	head = tail = NULL;
}

/**
*	constructor - memory file
*/
TProp::TProp()
{
	local = true;
	loaded = false;
	memoryFile = true;

	strcpy(fileName, "/tmp/tprop.temp");
	head = tail = NULL;
}

/**
*	destructor
*/
TProp::~TProp()
{
	if	( loaded && !memoryFile )	{
		storeToFile(fileName);
		clear();
		}
}

/**
*	return a local-for-the-user filename 
*
*	@param base is the basic part of the filename (use application's name)
*	@return a local-for-the-user filename 
*/
char *TProp::makeUserFile(const char *base)
{
	static char temp[PATH_MAX];

	sprintf(temp, "%s/.%s.tprop", getenv("HOME"), base);
	return temp;
}

/**
*	returns the section-object which had the specified name
*
*	@param name is the section-name
*	@return if found the section-object; otherwise returns NULL
*/
TProp::TSection* TProp::findSection(const char *name) const
{
	TProp::TSection	*curSec;

	curSec = head;
	while ( curSec )	{
		if	( strcmp(curSec->getName(), name) == 0 )
			return curSec;	// found
		curSec = curSec->getNextPtr();
		}
	return NULL;
}

/**
*	adds a new section
*
*	@param name is the section-name
*	@return the new section-object
*/
TProp::TSection* TProp::addSection(const char *name)
{
	TProp::TSection	*curSec;

	curSec = new TProp::TSection(name);

	if	( !head )
		head = tail = curSec;
	else	{
		tail->setNextPtr(curSec);
		tail = curSec; 
		}
	return curSec;
}

/**
*	checks if 'ch' is a valid variable-name character
*
*	@param ch the character to check
*	@return true if ch is valid
*/
bool	TProp::isValidVarNameChar(int ch) const
{
	if	( isalnum(ch) )
		return true;
	if	( strchr("_-+*{}()", ch) )
		return true;
	if	( ch > 0x7f )	// non-latin
		return true;
	return false;
}

/**
*	clears the section-list
*/
void	TProp::clear()
{
	TProp::TSection	*sec, *prevSec;

	sec = findSection("default");
	while ( sec )	{
		prevSec = sec;
		sec = sec->getNextPtr();

		delete prevSec;
		}
	head = tail = NULL;
}

/**
*	loads data from file (private)
*
*	@param file is the filename
*	@return true on success
*/
bool	TProp::restoreFromFile(const char *file)
{
	FILE	*fp;
	char	buf[1024];
	char	*p, *ps;
	TProp::TSection	*sec;

	sec = addSection("default");

	fp = fopen(file, "rt");
	if	( fp )	{
	   	while ( fgets(buf, 1024, fp) )	{
			// remove \n
			if	( buf[strlen(buf)-1] == '\n' )
		   		buf[strlen(buf)-1] = '\0';

			// skip spaces
	   		p = buf;
	   		while ( *p == ' ' || *p == '\t' )	p ++;
	   		ps = p;

	   		switch ( *p )	{
	   		case	'[':	// change section
	   			p ++;
	   			ps = p;
	   			p = strchr(p, ']');
	   			if	( !p )	// error, missing ']'
	   				sec->addText(buf);
	   			else	{
	   				*p = '\0';	// ps = section name
	   				sec = findSection(ps);
	   				if	( !sec )
	   					sec = addSection(ps);
	   				}
	   			break;

	   		case	'#':	// comment
	   			sec->addText(buf);
	   			break;

	   		default:		// check for variable
	   			p = strchr(buf, '=');
	   			if	( p )	{	// it is a variable
	   				char	*varName, *varValue;

	   				*p = '\0';	p ++;
	   				varName  = buf;
	   				varValue = p;

	   				// trim varName
	   				while ( *varName == ' ' || *varName == '\t' )	varName ++;
	   				p = varName;
	   				while ( isValidVarNameChar(*p) )	p ++;
	   				*p = '\0';

	   				if	( strlen(varName) )	{	
	   					// varValue: to trim or not to trim ?
	   					TProp::TSection::TVar	*var;

	   					var = sec->findVar(varName);
	   					if	( var )
	   						var->setValue(varValue);
	   					else
	   						var = sec->addVar(varName, varValue);
	   					}
	   				//
	   				//else	// it is a flag or comment ?
	   				else	{
	   					if	( strlen(ps) )	{
	   						char	*tmp = new char[strlen(buf)+4];
	   						sprintf(tmp, "# %s\n", buf);
	   						sec->addText(tmp);

	   						delete[] tmp;
	   						}
	   					else	// empty line
	   						sec->addText(buf);
	   					}
	   				}
	   			else	// add it as comment
	   				sec->addText(buf);
	   			}

	   		}	// while

	   	fclose(fp);
	   	}
	else
		return false;

	return true;
}

/**
*	Opens the configuration file and loads it to memory
*
*	TODO: add a locking mechanism
*/
void	TProp::lockConf()
{

	if	( loaded )
		return;

	if	( !memoryFile )		// not a memory file
		restoreFromFile(fileName);

	loaded = true;
}

/**
*	saves to file (private)
*
*	@param file is the filename
*	@return true on success
*/
bool	TProp::storeToFile(const char *file)
{
	FILE	*fp;
	TProp::TSection	*sec;
	bool	defsec = true;

	fp = fopen(file, "wt");
	if	( fp )	{
		sec = findSection("default");
	   	while ( sec )	{
	   		// write the section name
	   		if	( defsec )		// the default section has no label
	   			defsec = false;
	   		else	
	   			fprintf(fp, "[%s]\n", sec->getName());

	   		// the variables and/or the comments
	   		TProp::TSection::TNode	*node;

	   		node = sec->getHeadPtr();
	   		while ( node )	{
	   			if	( node->isVar() )	
	   				fprintf(fp, "%s=%s\n", node->getVarPtr()->getName(), node->getVarPtr()->getValue());
	   			else
	   				fprintf(fp, "%s\n", node->getTextPtr());

	   			// next node
	   			node = node->getNextPtr();
	   			}

	   		// next section
	   		sec = sec->getNextPtr();
	   		}

	   	fclose(fp);
	   	}
	else
		return false;

	return true;
}

/**
*	Re-writes the configuration file
*/
void	TProp::unlockConf(bool save)
{
	if	( !loaded )
		return;

	if	( save )	{
		if	( !memoryFile && !local )	
			storeToFile(fileName);
		}

	if	( !local )	{
		clear();
		loaded = false;
		}
}

/**
*	stores a string value
*
*	@param section is the section (you can use NULL for default)
*	@param varName is the variable's name
*	@param varVal  is the value
*/
void	TProp::set(const char *section, const char *varName, const char *varVal)
{
	const char				*secName;
	TProp::TSection			*curSec;
	TProp::TSection::TVar	*curVar;

	if	( !section )
		secName = "default";
	else
		secName = section;

	// load file and lock it
	lockConf();

	// find section
	curSec = findSection(secName);
	if	( !curSec )	
		curSec = addSection(secName);

	// find variable
	curVar = curSec->findVar(varName);
	if	( !curVar )	// new variable ?
		curVar = curSec->addVar(varName, varVal);
	else
		curVar->setValue(varVal);

	// write file and unlock it
	unlockConf(true);
}

/**
*	stores an integer value
*
*	@param section is the section (you can use NULL for default)
*	@param varName is the variable's name
*	@param varVal  is the value
*/
void	TProp::set(const char *section, const char *varName, int varValue)
{
	char	buf[32];

	sprintf(buf, "%d", varValue);
	set(section, varName, buf);
}

/**
*	stores a double value
*
*	@param section is the section (you can use NULL for default)
*	@param varName is the variable's name
*	@param varVal  is the value
*/
void	TProp::set(const char *section, const char *varName, double varValue)
{
	char	buf[64];

	sprintf(buf, "%f", varValue);
	set(section, varName, buf);
}

/**
*	return a value of a variable as string
*
*	@param section is the section (you can use NULL for default)
*	@param varName is the variable's name
*	@param varVal  is the value
*/
const char*	TProp::get(const char *section, const char *varName, const char *defaultValue)
{
	const char				*secName;
	TProp::TSection			*curSec;
	TProp::TSection::TVar	*curVar;

	if	( !section )
		secName = "default";
	else
		secName = section;

	// default
	strcpy(rvbuf, defaultValue);

	// load file and lock it
	lockConf();

	// find section
	curSec = findSection(secName);
	if	( curSec )	{
		// find variable
		curVar = curSec->findVar(varName);
		if	( curVar )	{
			if	( local )	
				return curVar->getValue();		// unlock - not needed
			else	
				strcpy(rvbuf, curVar->getValue());
			}
		}

	// unlock it
	unlockConf(false);
	return rvbuf;
}

/**
*	return a value of a variable as integer
*
*	@param section is the section (you can use NULL for default)
*	@param varName is the variable's name
*	@param varVal  is the value
*/
int		TProp::getInt(const char *section, const char *varName, int defVal)
{
	const char	*p;
	char		buf[32];
	
	sprintf(buf, "%d", defVal);
	p = get(section, varName, buf);
	return atoi(p);
}

/**
*	return a value of a variable as double
*
*	@param section is the section (you can use NULL for default)
*	@param varName is the variable's name
*	@param varVal  is the value
*/
double	TProp::getFlt(const char *section, const char *varName, double defVal)
{
	const char	*p;
	char		buf[64];
	
	sprintf(buf, "%f", defVal);
	p = get(section, varName, buf);
	return atof(p);
}

/**
*	loads data from a file
*	note: this is not rq if !local
*
*	@param file is the filename	
*	@return true on success
*/
bool	TProp::load(const char *file)
{
	clear();
	loaded = false;
	if	( file )
		return restoreFromFile(file);
	return restoreFromFile(fileName);
}

/**
*	saves data to a file
*	note: this is not rq if !local
*
*	@param file is the filename	
*	@return true on success
*/
bool	TProp::save(const char *file)
{
	if	( file )
		return storeToFile(file);
	return storeToFile(fileName);
}

/**
*	dump everything (debug)
*/
void	TProp::dump()
{
	TProp::TSection	*sec;
	bool	defsec = true;
	FILE	*fp = stdout;

	if	( (sec = findSection("default")) == NULL )
		printf("default section, not found\n");

   	while ( sec )	{
   		// write the section name
   		if	( defsec )		// the default section has no label
   			defsec = false;
   		else	
   			fprintf(fp, "[%s]\n", sec->getName());

   		// the variables and/or the comments
   		TProp::TSection::TNode	*node;

   		node = sec->getHeadPtr();
   		while ( node )	{
   			if	( node->isVar() )	
   				fprintf(fp, "%s=%s\n", node->getVarPtr()->getName(), node->getVarPtr()->getValue());
   			else
   				fprintf(fp, "%s\n", node->getTextPtr());

   			// next node
   			node = node->getNextPtr();
   			}

   		// next section
   		sec = sec->getNextPtr();
	   	}
}

