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

#if !defined(__TProp_H)
#define __TProp_H

#include <stdio.h>

class TProp	{
private:
	// section node
	class TSection	{
	private:
		
		/////////////////////////////////
		// variable node
		class TVar	{
		private:
			char	*name;
			char	*value;

		public:
			TVar(const char *varName, const char *varValue);
			virtual ~TVar();

			const char *getValue() const	{ return value; }
			const char *getName() const		{ return name; }

			void		setValue(const char *val);
			void		setValue(int val);
			void		setValue(double val);
			};

		/////////////////////////////////
		// file node types
		enum fileNodeType_t	{
			fnt_var,			// the node contains a variable
			fnt_text			// the node contains a generic text
			};

		/////////////////////////////////
		// file node
		class TNode	{
		private:
			fileNodeType_t	type;
			TVar	   	*var;
			char	   	*text;
			TNode		*next;

		public:
			TNode(fileNodeType_t type, const char *varName, const char *varValue = NULL);

			TProp::TSection::TNode*	getNextPtr() const		{ return next; }
			void					setNextPtr(TProp::TSection::TNode *node)	{ next = node; }
			TProp::TSection::TVar*	getVarPtr() const		{ return var; }
			const char*				getTextPtr() const		{ return text; }

			int		isVar() const			{ return (type == fnt_var); }
			int		isText() const			{ return (type == fnt_text); }

			virtual ~TNode();
			};

		/////////////////////////////////

		char	*name;			// section name
		TNode	*head, *tail;	// head,tail - list pointers

		TSection *next;

		void	clear();

	public:
		TSection(const char *secName);
		virtual ~TSection();

		TProp::TSection::TNode*	getHeadPtr() const		{ return head; }
		TProp::TSection*		getNextPtr() const		{ return next; }
		void					setNextPtr(TProp::TSection *node)	{ next = node; }

		const char *			getName() const		{ return name; }

		TProp::TSection::TVar* 	findVar(const char *name) const;
		TProp::TSection::TVar* 	addVar(const char *name, const char *val);
		TProp::TSection::TNode*	addText(const char *text);
		};

	//
	char	fileName[1024];

	// local buffers, using instead of static
	// not multithread but different per object
	mutable char rvbuf[1024];

	bool	local;				// true = don't read/write configuration all the time (only this application is using it)
	bool	loaded;				// true = if 'local', the file is already in memory
	bool	memoryFile;			// true = memory file (save/load not allowed)

	TProp::TSection	*head, *tail;

	TProp::TSection	*findSection(const char *name) const;
	TProp::TSection	*addSection(const char *name);

	void	clear();
	void	lockConf();
	void	unlockConf(bool save);

	bool	isValidVarNameChar(int ch) const;
	bool	storeToFile(const char *file);
	bool	restoreFromFile(const char *file);

public:
	TProp(const char *file, bool blocal=true);	 	// constructor - local mode/shared mode
	TProp(); 										// constructor - memoryFile
	virtual ~TProp();								// destructor

	// returns a local-for-the-user filename 
	static char *makeUserFile(const char *base);	

	// set values 
	void	set(const char *section, const char *varName, const char *varValue);	// string
	void	set(const char *section, const char *varName, int varValue);			// int
	void	set(const char *section, const char *varName, double varValue);			// double
	void	set(const char *section, const char *varName, bool varValue)			// bool
			{ set(section, varName, (varValue) ? 1 : 0); }

	void	setInt(const char *section, const char *varName, int varValue)			// int
			{ set(section, varName, varValue); }
	void	setFlt(const char *section, const char *varName, double varValue)		// double
			{ set(section, varName, varValue); }
	void	setBool(const char *section, const char *varName, bool varValue)		// bool
			{ set(section, varName, varValue); }

	// get values
	const char*	get(const char *section, const char *varName, const char *defaultValue); // default=string

	const char*	getStr(const char *section, const char *varName, const char *defVal)	// string
			{ return get(section, varName, defVal); }
			
	int		getInt(const char *section, const char *varName, int defVal);				// int
	double	getFlt(const char *section, const char *varName, double defVal);			// double
	bool	getBool(const char *section, const char *varName, bool defVal)				// bool
			{ return (getInt(section, varName, defVal) != 0); }

	// if memory-file then that functions are usefull :)
	bool	load(const char *file = NULL);
	bool	save(const char *file = NULL);

	// debug
	void	dump();
	};

#endif

