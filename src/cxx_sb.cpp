/**
*	SmallBASIC, high-level C++ shell
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*
*	Nicholas Christopoulos
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "cxx_sb.hpp"

#if !defined(O_BINARY)
	#define O_BINARY	0
#endif

//static int SmallBASIC::tempCount = 0;

// default constructor
SmallBASIC::SmallBASIC()
{
	lastCode = NULL;		// last program code
	lastCWD = new char[1024];   // current working directory
//	getcwd(lastCWD, 1024);
	strcpy(lastCWD, ".");
	errLine = 0;			// last error line (user lines starting from 1)
	errMessage = NULL;		// last error message

	opt_ide = 1;			// setup IDE flag
	opt_quite = 1;			// setup quite flag
}

// destructor
SmallBASIC::~SmallBASIC()
{
	if  ( lastCode )	delete[] lastCode;
	if  ( errMessage )  delete[] errMessage;
	delete lastCWD;
}

// strdup, but using the 'new' keyword
char *SmallBASIC::csdup(const char *source) const
{
	int	 len = strlen(source);
	char	*p = new char[len+1];

	strcpy(p, source);
	return p;
}

// returns a temporary file name
void	SmallBASIC::getTemp(char *buf)
{
	#if defined(_DOS)
	sprintf(buf, "c:\\sbide-%d.tmp", ++ tempCount);
	#elif defined(_Win32)
	sprintf(buf, "c:\\sbide-%d.tmp", ++ tempCount);
	#else
	sprintf(buf, "/tmp/sbide-%d-%d.tmp", getpid(), ++ tempCount);
	#endif
}

// writes a text to a file
bool	SmallBASIC::setFileText(const char *fileName, const char *text)
{
	int	h;

	h = open(fileName, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, 0660);
	if  ( h > -1 )  {
		write(h, text, strlen(text));
		close(h);
		return true;
		}
	return false;
}

// reads & return the text of a file
char	*SmallBASIC::getFileText(const char *fileName)
{
	int	 	h;
	char	*buf;

	h = open(fileName, O_RDWR | O_BINARY); // O_RDONLY
	if  ( h > -1 ) {
		long	size;

		// file size
		size = lseek(h, 0, SEEK_END);
		lseek(h, 0, SEEK_SET);
		
		// setup code
		buf = new char[size+1];
		memset(buf, 0, size+1);

		// load
		read(h, buf, size);

		//
		close(h);
		return buf;
		}
	return NULL;
}

// setup error
void	SmallBASIC::errSet(int line, const char *message)
{
	errLine = line;

	if  ( errMessage )  delete[] errMessage;
	if	( message )
		errMessage = csdup(message);
	else
		errMessage = NULL;
}

// setup current working directory
bool	SmallBASIC::setCWD(const char *dir)
{
	if  ( access(dir, F_OK|R_OK|W_OK) == 0 ) {
		strcpy(lastCWD, dir);
		return true;
		}
	return false;
}

// load a file
bool	SmallBASIC::load(const char *file)
{
	char	*new_code;

	new_code = getFileText(file);
	if	( new_code )	{
		if  ( lastCode )	delete[] lastCode;
		lastCode = new_code;
		errSet(0, NULL);
		return true;
		}
	return false;
}

// prepare code
bool	SmallBASIC::setCode(const char *code, char *fileName)
{
	const char  *source;

	if  ( code )
		source = code;
	else	{
		if  ( lastCode )
			source = lastCode;
		else
			return false;   // failed because there is no code
		}

	// create a temporary and store the code
	// (maybe later I'll update compiler to work with string instead of file)
	getTemp(fileName);
	setFileText(fileName, source);

	// run
	errSet(0, NULL);
	return true;
}

// compile
bool	SmallBASIC::compile(const char *program)
{
	char		fileName[1024];

	if	( !setCode(program, fileName) )
		return false;

	//
	opt_syntaxcheck = 1;
	errSet(0, NULL);
	sbasic_main(fileName);
	if	( gsb_last_line )	
		errSet(gsb_last_line, gsb_last_errmsg);
	opt_syntaxcheck = 0;

	//
	remove(fileName);
	if	( errLine )
		return false;
	return true;
}

// decompile
const char *SmallBASIC::decompile(const char *program)
{
	char	input[1024];
	char	*buf = NULL;

	if	( !setCode(program, input) )
		return NULL;

	//
	opt_decomp = 1;
	errSet(0, NULL);
	sbasic_main(input);
	if	( gsb_last_line )	{
		errSet(gsb_last_line, gsb_last_errmsg);
		buf = NULL;
		}
//	else	{
//		buf = getFileText(output);
//		}
	opt_decomp = 0;

	remove(input);
	return buf;
}

// compile & execute
bool	SmallBASIC::exec(const char *program)
{
	char		fileName[1024];

	if	( !setCode(program, fileName) )
		return false;

	//
	errSet(0, NULL);
	sbasic_main(fileName);
	if	( gsb_last_line )	
		errSet(gsb_last_line, gsb_last_errmsg);

	//
	remove(fileName);
	if	( errLine )
		return false;
	return true;
}

// execution status
int		SmallBASIC::execStatus()
{
	if	( prog_error )	
		return 0;	// stopped
	if	( prog_line )
		return prog_line;	// current execution line
	return comp_line;	// current compile line
}

// pauses the execution
void	SmallBASIC::execPause()
{
}

// stops the execution
void	SmallBASIC::execBreak()
{
	brun_stop();
}

