/*
*	SmallBASIC run-time errors
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*
*	Nicholas Christopoulos
*/

#include "smbas.h"
#include "pproc.h"
#if !defined(_PalmOS)
#include <string.h>
#include <errno.h>
#endif

/*
*/
void	err_common_msg(const char *seg, const char *file, int line, const char *descr) SEC(TRASH);
void	err_common_msg(const char *seg, const char *file, int line, const char *descr)
{
	gsb_last_line = line;
	gsb_last_error = prog_error;
	strcpy(gsb_last_file, file);
	strncpy(prog_errmsg, descr, SB_ERRMSG_SIZE);
	prog_errmsg[SB_ERRMSG_SIZE] = '\0';
	strcpy(gsb_last_errmsg, prog_errmsg);

	if	( opt_ide == IDE_NONE )	{
		#if defined(_UnixOS)
		if ( !isatty (STDOUT_FILENO) ) {
			// hm... out or err ?
			fprintf(stdout, "\n* %s-ERROR AT %s:%d # %s\n", seg, file, line, descr);
			}
		else	{
		#endif

		dev_printf("\n\033[0m\033[80m\n");
		dev_printf("\033[7m * %s-ERROR AT %s:%d * \033[0m\a\n\n", seg, file, line);
		dev_printf("\033[4mDescription:\033[0m\n%s\n", descr);
		#if defined(_PalmOS)
		dev_printf("\n\033[4mPress '.' to return...\033[0m\n");
		#endif
		dev_printf("\033[80m\033[0m");

		#if defined(_UnixOS)
			}
		#endif
		}
}

/*
*	raise a compiler error
*/
void	sc_raise2(const char *sec, int scline, const char *buff)
{
	prog_error = 0x40;
	err_common_msg("COMP", sec, scline, buff);
}

/*
*	run-time error
*/
void	rt_raise(const char *fmt, ...)
{
	char	*buff;
	va_list ap;

	prog_error = 0x80;

	va_start(ap, fmt);
	buff = tmp_alloc(SB_TEXTLINE_SIZE+1);
	#if defined(_PalmOS)
	StrVPrintF(buff, fmt, ap);
	#else
	vsprintf(buff, fmt, ap);
	#endif
	va_end(ap);
	
	err_common_msg("RT", prog_file, prog_line, buff);

	tmp_free(buff);
}

/* ERROR MESSAGES */
void	err_file(dword code)
{
	#if defined(_PalmOS)
	switch ( code )	{
	case	fileErrMemError:
		rt_raise("FS: Out of memory");
		break;
	case	fileErrInvalidParam:
		rt_raise("FS: Invalid parameter");
		break;
	case	fileErrCorruptFile:
		rt_raise("FS: File is corrupted or invalid");
		break;
	case	fileErrNotFound:
		rt_raise("FS: File not found");
		break;
	case	fileErrTypeCreatorMismatch:
		rt_raise("FS: Type or creator not what was specified");
		break;
	case	fileErrReplaceError:
		rt_raise("FS: Coundn't replace existing file");
		break;
	case	fileErrCreateError:
		rt_raise("FS: Couldn't create new file");
		break;
	case	fileErrOpenError:
		rt_raise("FS: Generic open error");
		break;
	case	fileErrInUse:
		rt_raise("FS: File is in use");
		break;
	case	fileErrReadOnly:
		rt_raise("FS: File is read-only");
		break;
	case	fileErrInvalidDescriptor:
		rt_raise("FS: Invalid file handle");
		break;
	case	fileErrCloseError:
		rt_raise("FS: Error closing file");
		break;
	case	fileErrOutOfBounds:
		rt_raise("FS: Past end of file");
		break;
	case	fileErrPermissionDenied:
		rt_raise("FS: Access denied");
		break;
	case	fileErrIOError:
		rt_raise("FS: Generic I/O error");
		break;
	case	fileErrEOF:
		rt_raise("FS: End-Of-File error!");
		break;
	case	fileErrNotStream:
		rt_raise("FS: File is not a stream");
		}
	#else
	char	buf[1024], *p;

	strcpy(buf, strerror(code));
	p = buf;
	while ( *p )	{
		*p = to_upper(*p);
		p ++;
		}
	rt_raise("FS(%d): %s", code, buf);
	#endif
}


#if defined(OS_LIMITED)
void	err_missing_rp(void)		{	rt_raise("Missing ')' OR invalid number of parameters"); }
void	err_matdim(void)			{	rt_raise("Matrix dimension error"); 					}
void	err_syntax(void) 	   		{ 	rt_raise("Syntax error");  						}
void	err_syntaxsep(int c)		{ 	rt_raise("Missing separator '%c'", c);  		}
void	err_parm_num(void)			{	rt_raise("Error number of parameters");			}
#endif

void	err_stackoverflow(void)		{	rt_raise("Stack overflow");						}
void	err_stackunderflow(void)	{	rt_raise("Stack underflow");					}
void	err_stackmess()				{ 	rt_raise("Stack mess!");  						}

void	err_arrmis_lp(void)			{	rt_raise("Array: Missing '('"); 				}
void	err_arrmis_rp(void)			{	rt_raise("Array: Missing ')'"); 				}
void	err_arridx(void)			{	rt_raise("Array: Index out of range");			}
void	err_typemismatch(void)		{	rt_raise("Type mismatch");						}
void	err_argerr(void)			{	rt_raise("Invalid parameter");					}

void	err_varisarray(void)		{	rt_raise("Eval: Variable is an array"); 		}
void	err_varisnotarray(void)		{	rt_raise("Eval: Variable is NOT an array (Use DIM)"); }
void	err_vararridx(void)			{	rt_raise("Eval: Array: Index out of range"); 	}
void	err_varnotnum(void)			{	rt_raise("Eval: Not a number"); 				}
void	err_evsyntax(void)			{	rt_raise("Eval: Syntax error"); 				}
void	err_evtype(void)			{	rt_raise("Eval: Type mismatch"); 				}
void	err_evargerr(void)			{	rt_raise("Eval: Invalid parameter"); 			}
void	err_unsup(void)				{	rt_raise("Unsupported"); 						}
void	err_const(void)				{	rt_raise("LET: Cannot change a constant");		}
void	err_notavar(void)			{	rt_raise("Not a variable");						}

void	err_notarray(void)			{	rt_raise("NOT an array OR function"); 			}
void	err_out_of_range(void)		{	rt_raise("Out of range"); 						}
void	err_missing_sep(void)		{	rt_raise("Missing separator OR parenthesis"); 	}
void	err_division_by_zero(void)	{	rt_raise("Division by zero"); 					}
void	err_matop(void)				{	rt_raise("Operator NOT allowed"); 				}
void	err_matsig(void)			{	rt_raise("Matrix singular"); 					}
void	err_missing_lp(void)		{	rt_raise("Missing '('"); 						}

void	err_parfmt(const char *fmt)	{	rt_raise("Parameters count/format error (%s)", fmt); }
void	err_parm_byref(int n)		{ 	rt_raise("Parameter %d cannot BYREF", n);  		}

void	err_stridx(void)			{	rt_raise("String: Index out of range"); 		}
void	err_fopen(void)				{	rt_raise("VFS: Bad file number (Use OPEN)");	}
void	err_syntaxanysep(const char *seps)
									{	rt_raise("No separator found (missing %s)", seps); }
void	err_parsepoly(int idx, int mark)		{	rt_raise("Parsing polyline: type mismatch! (element: %d, info: %d)", idx, mark); }

void	err_bfn_err(long code)
		{	rt_raise("Unsupported buildin function call %ld, please report this bug", code); }

void	err_gpf(addr_t addr, int bc)
{
	dev_printf("\n\aOUT OF ADDRESS SPACE\n");
	rt_raise("SEG:CODE[%d]=%02X", addr, bc);
}

void	err_pcode_err(long pcode)
{	rt_raise("Unsupported buildin procedure call %ld, please report this bug", pcode);	}

void	err_chain_err(const char *file)
									{	rt_raise("CHAIN: FILE '%s' DOES NOT EXIST", file);	}

void	err_run_err(const char *file)
									{	rt_raise("RUN/EXEC\"%s\" FAILED", file);	}

void	err_invkw(addr_t addr, byte code)
									{		rt_raise("PARAM COUNT ERROR @%d=%X", addr, (int) code);	}

/*
*	the DONE message
*/
void	inf_done()
{
	#if defined(_UnixOS)
	if ( !isatty (STDOUT_FILENO) ) 
		fprintf(stdout, "\n* DONE *\n");
	else {
	#endif

	#if defined(_PalmOS)
	dev_print("\a\n");
	dev_settextcolor(15,0);
	dev_setxy(0,os_graf_my-9);
	dev_print("\033[91m\033[0m\033[7m * DONE * - Press '.' to return\4 \033[80m\033[0m");
	#else
	dev_print("\n\033[0m\033[80m\a\033[7m * DONE * \033[0m\n");
	#endif

	#if defined(_UnixOS)
		}
	#endif
}

/*
*	the BREAK message
*/
void	inf_break(int pline)
{
	#if defined(_UnixOS)
	if ( !isatty (STDOUT_FILENO) ) 
		fprintf(stdout, "\n* BREAK AT %d *\n", pline);
	else {
	#endif

	dev_settextcolor(15, 0);
	#if defined(_PalmOS)
	dev_print("\a\n");
	dev_setxy(0,os_graf_my-9);
	dev_printf("\033[91m\033[0m\033[7m * BREAK AT %d * - Press '.' to return\4 \033[80m\033[0m", pline);
	#else
	dev_printf("\n\033[0m\033[80m\a\033[7m * BREAK AT LINE %d * \033[0m\n", pline);
	#endif

	#if defined(_UnixOS)
		}
	#endif
}

/*
*	if bytecode files are keeped; messages of the compiler why recompile is needed
*/
void	inf_comprq_dv()
{
	if	( !opt_quite )
		dev_print("Recompile: different version\n");
	else
		dev_print("(ver)\n");
}

void	inf_comprq_dt()
{
	if	( !opt_quite )
		dev_print("Recompile: source is newer\n");
	else
		dev_print("(mod)\n");
}

void	inf_comprq_prq()
{
	if	( !opt_quite )
		dev_print("Recompile: no binary file\n");
	else
		dev_print("(bin)\n");
}

/*
*	Low-battery event/signal
*/
void	inf_low_battery()
{
	dev_print("\n\n\a* ALARM: LOW BATTERY *\a\n\n");
}


