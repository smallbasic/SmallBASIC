/**
*	SmallBASIC IDE for Unix terminals
*
*	Copyright (C) 2002 Nicholas Christopoulos
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include "ted_dt.h"
#include "cxx_sb.hpp"

TedDT		*ted;
SmallBASIC	*sb;

static char	ide_opt_defpars[1024];	// default sbasic parameters
static int	last_err_line;
static char	last_err_msg[4096];

#define	SBIDE_VER	"0.1.0"

enum buff_t {
	bf_clip,
	bf_help,
	bf_out,
	bf_work,
	bf_notes,
	bf_user
	};

#if defined(_UnixOS)
void sigWINCH(int a)
{
	term_recalc_size();
	ted->key(TEDK_REFRESH);
}
#endif

/**
*	compilation/execution error
*/
void	beep_error()
{
	printf("\a");
}

/**
*	compilation finished
*/
void	beep_finished()
{
}

/**
*   execute a command and return its output.
*	on error returns NULL; otherwise the output as a newly created (by malloc()) string
*
*	On DOS we cannot use fork()... I am looking for a compatible way
*/
char	*ide_exec(const char *cmd, bool sendfile)
{
	char	*ret = NULL;
	char	fnin[1024];
	char	fnout[1024];
	char	buf[1024];
	FILE	*fp;

	#if defined(_DOS)
	sprintf(fnin,  "in.tmp");
	sprintf(fnout, "out.tmp");
	#else
	sprintf(fnin,  "/tmp/sbide.in.%d.tmp", getpid());
	sprintf(fnout, "/tmp/sbide.out.%d.tmp", getpid());
	#endif

	if	( sendfile )	{
		ted->save_to(fnin, ted->area());
		sprintf(buf, "%s < %s > %s", cmd, fnin, fnout);
		}
	else	
		sprintf(buf, "%s > %s", cmd, fnout);

	system(buf);

	// read the output
	fp = fopen(fnout, "r");
	if	( fp )	{
		int		size;

		fseek(fp, 0L, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		ret = (char *) malloc(size+2);
		fread(ret, size, 1, fp);
		ret[size] = '\n';
		ret[size+1] = '\0';
		fclose(fp);
		}

	// clean up
	remove(fnin);
	remove(fnout);

	return ret;
}

/**
*	compile
*/
void	sbcomp(const char *file)
{
	sb->load(file);
	if	( !sb->compile() )	{
		last_err_line = sb->errLine;
		sprintf(last_err_msg, "\033[22m\033[31mERR:%s @%d\033[1m", sb->errMessage, sb->errLine);
		// update ted
		ted->prompt(last_err_msg);
		ted->goto_line(last_err_line);
		ted->redraw_screen();

		// beep - error
		beep_error();
		}
	else	{
		last_err_line = 0;

		// update ted
		ted->prompt("no error :)");
		ted->refresh();

		// beep - finished
		beep_finished();
		}
}

/**
*	compile & run
*/
void	sbcrun(const char *file)
{
	sbcomp(file);
	if	( !last_err_line )	{
		term_settextcolor(7,0);
		term_cls();
		fflush(stdout);

		if	( !sb->exec() )	{
			last_err_line = sb->errLine;
			sprintf(last_err_msg, "\033[22m\033[31mERR:%s @%d\033[1m", sb->errMessage, sb->errLine);
			// update ted
			ted->prompt(last_err_msg);
			ted->goto_line(last_err_line);
			ted->redraw_screen();

			// beep - error
			beep_error();
			}
		else	{
			last_err_line = 0;

			// update ted
			ted->prompt("no error :)");
			ted->refresh();

			// beep - finished
			beep_finished();
			}

		term_setxy(0,term_rows()-1);
		printf("\033[0m\033[K\033[1m\033[32mSBIDE: Press any key...");
		fflush(stdout);

		ted->redraw_screen();	ted->refresh();
		}
}

/**
*	execute macro
*/
void	execmacro()
{
	char	buf[1024];
	char	cmd[1024];
	char	rscr[1024];
	char	*text;

	ted->ask("execute shell command:", buf, 1024, "");
	if	( strlen(buf) )	{
		text = ide_exec(buf, false);
		if	( text )	{
			ted->set_text(text, bf_work);
			free(text);
			ted->refresh();
			}
		else	{
			ted->prompt("'%s': can't run", buf);
			ted->refresh();
			}
		}
}

/*
*/
int		main(int argc, char *argv[])
{
	int		c, tc, i;
	char	filename[1024];

	term_init();

	#if defined(_UnixOS)
	signal(SIGWINCH, sigWINCH);
	#endif

	ted = new TedDT();
	sb  = new SmallBASIC();

	// create local dir
	sprintf(filename, "%s/%s", getenv("HOME"), ".sbide");
	if	( access(filename, F_OK) )
		mkdir(filename, 0700);

	// read SB syntax
	sprintf(filename, "/usr/lib/sbasic/.sbwords");	// TODO:
	if	( access(filename, R_OK) == 0 )	
		ted->add_stx(filename, ".bas");

	//
	strcpy(ide_opt_defpars, "");

	term_cls();
	ted->paint();

	// load user files
	for ( i = 1; i < argc; i ++ )	
		ted->load(argv[i], bf_user+(i-1));

	// load default pages
	#if defined(_DOS)
	ted->load("help.sbi", bf_help);
	ted->load("output.sbi", bf_out);
	ted->load("notes.sbi", bf_notes);
	ted->load("workpad.sbi", bf_work);
	#else
	ted->load("/usr/lib/sbasic/.help", bf_help);
	sprintf(filename, "%s/%s", getenv("HOME"), ".sbide/.output");
	ted->load(filename, bf_out);
	sprintf(filename, "%s/%s", getenv("HOME"), ".sbide/.notes");
	ted->load(filename, bf_notes);
	sprintf(filename, "%s/%s", getenv("HOME"), ".sbide/.workpad");
	ted->load(filename, bf_work);
	#endif

	//
	ted->activate(bf_user);

	// main
	ted->prompt("sbide ver %s - press <shift+F1> for editor's help page", SBIDE_VER);
	ted->refresh();
	while ( !ted->finished )	{
		c = ted->get_row_ch();
		tc = ted->translate(c);

		// keys
		switch ( c )	{
		case	SB_KEY_SF(1):	// help
			ted->activate(bf_help);
			break;
		case	SB_KEY_F(9):	// macro
			execmacro();
			break;
		case	SB_KEY_F(11):	// compile
			ted->save(NULL, ted->area());
			strcpy(filename, ted->get_curr_name());
			sbcomp(filename);
			break;
		case	SB_KEY_F(12):	// compile & run
			ted->save(NULL, ted->area());
			strcpy(filename, ted->get_curr_name());
			sbcrun(filename);
			break;
		default:
			ted->key(tc);
			}
		}

	// close
	delete ted;
	delete sb;

	// bye bye
	term_setxy(0, term_rows()-1);
	printf("\n\033[0m\033[K");
//	printf("\n\033[0m\033[Ksbide terminated.\n");
	term_setcursor(1);
	fflush(stdout);
	term_restore();
	return 1;
}

