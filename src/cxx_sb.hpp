/**
*	SmallBASIC, high-level C++ shell
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*
*	Nicholas Christopoulos
*
*	Warning: SB is not a multithread class
*/

#if !defined(__sb_cxx_h)
#define __sb_cxx_h

#include "sbapp.h"

/**
*	SmallBASIC C++ shell
*/
class SmallBASIC {
private:	// private - members
	char	*lastCode;			// last program code
	char	*lastCWD;			// current working directory
	/* static */ int 	tempCount;		// temporary file count
		
public:		// public - members
	int		errLine;			// last error line
	char	*errFile;			// last file (where the error happend)
	char	*errMessage;		// last error message

private:	// private - methods
	void	errSet(int line, const char *message);		// setup error info
	char*	csdup(const char *source) const;			// strdup
	void	getTemp(char *fileName);					// returns a temporary filename
	bool    setFileText(const char *fileName, const char *text);	// writes a text to a file
	char	*getFileText(const char *fileName);			// returns the text of a file
	bool	setCode(const char *code, char *fileName);	// prepare code for compile or execute

public:		// public - methods
	SmallBASIC();
	virtual ~SmallBASIC();

	/**
	*	set current working directory
	*
	*	@return true on success
	*/
	bool	setCWD(const char *dir);

	/**
	*	loads a file
	*
	*	@return true on success
	*/
	bool	load(const char *file);

	/**
	*	compiles the program
	*
	*	@program the code; if it is omitted then it uses the last code (load())
	*	@return true on success; otherwise returns false (errLine and errMessage are defined)
	*/
	bool	compile(const char *program = NULL);

	/**
	*	compiles & decompiles the program
	*
	*	@program the code; if it is omitted then it uses the last code (load())
	*	@return NULL on error; otherwise a new allocated string with the output
	*/
	const char *decompile(const char *program = NULL);

	/**
	*	compiles & executes the program
	*
	*	@program the code; if it is omitted then it uses the last code (load()/compile())
	*	@return true on success; otherwise returns false (errLine and errMessage are defined)
	*/
	bool	exec(const char *program = NULL);

	/**
	*	@return execution status (the line number, 0 = stopped)
	*/
	int		execStatus();

	/**
	*	pauses the execution
	*/
	void	execPause();	// todo

	/**
	*	stops the execution
	*/
	void	execBreak();
	};

#endif


