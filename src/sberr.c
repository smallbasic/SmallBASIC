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
#include "messages.h"
#if !defined(_PalmOS)
#include <string.h>
#include <errno.h>
#endif

#if defined(_WinBCB)
extern void bcb_comp(int pass, int pmin, int pmax);	// Win32GUI progress
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
        #if defined(_UnixOS) && !defined(_FLTK)
		if ( !isatty (STDOUT_FILENO) ) {
			// hm... out or err ?
			fprintf(stdout, "\n* %s-%s %s:%d # %s\n", seg, WORD_ERROR_AT, file, line, descr);
			}
		else	{
		#endif

		dev_printf("\n\033[0m\033[80m\n");
		dev_printf("\033[7m * %s-%s %s:%d * \033[0m\a\n\n", seg, WORD_ERROR_AT, file, line);
		dev_printf("\033[4m%s:\033[0m\n%s\n", WORD_DESCRIPTION, descr);
		#if defined(_PalmOS)
		dev_printf("\n\033[4mPress '.' to return...\033[0m\n");
		#endif
		dev_printf("\033[80m\033[0m");

		#if defined(_UnixOS) && !defined(_FLTK)
			}
		#endif
    }

	#if defined(_WinBCB)
	bcb_comp(-1, line, 0);
	#endif
}

/*
*	raise a compiler error
*/
void	sc_raise2(const char *sec, int scline, const char *buff)
{
	prog_error = 0x40;
	err_common_msg(WORD_COMP, sec, scline, buff);
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
	
	err_common_msg(WORD_RTE, prog_file, prog_line, buff);

	tmp_free(buff);
}

/* ERROR MESSAGES */
void	err_file(dword code)
{
	#if defined(_PalmOS)
	switch ( code )	{
	case	fileErrMemError:
		rt_raise(FSERR_OUT_OF_MEMORY);
		break;
	case	fileErrInvalidParam:
		rt_raise(FSERR_INVALID_PARAMETER);
		break;
	case	fileErrCorruptFile:
		rt_raise(FSERR_CORRUPTED);
		break;
	case	fileErrNotFound:
		rt_raise(FSERR_NOT_FOUND);
		break;
	case	fileErrTypeCreatorMismatch:
		rt_raise(FSERR_TYPE_MSM);
		break;
	case	fileErrReplaceError:
		rt_raise(FSERR_OVERWRITE);
		break;
	case	fileErrCreateError:
		rt_raise(FSERR_CREATE);
		break;
	case	fileErrOpenError:
		rt_raise(FSERR_OPEN);
		break;
	case	fileErrInUse:
		rt_raise(FSERR_USED);
		break;
	case	fileErrReadOnly:
		rt_raise(FSERR_READ_ONLY);
		break;
	case	fileErrInvalidDescriptor:
		rt_raise(FSERR_HANDLE);
		break;
	case	fileErrCloseError:
		rt_raise(FSERR_CLOSE);
		break;
	case	fileErrOutOfBounds:
		rt_raise(FSERR_EOF);
		break;
	case	fileErrPermissionDenied:
		rt_raise(FSERR_ACCESS);
		break;
	case	fileErrIOError:
		rt_raise(FSERR_GENERIC);
		break;
	case	fileErrEOF:
		rt_raise(FSERR_PALM_EOF);
		break;
	case	fileErrNotStream:
		rt_raise(FSERR_ANOMALO);
		}
	#else
	char	buf[1024], *p;

	strcpy(buf, strerror(code));
	p = buf;
	while ( *p )	{
		*p = to_upper(*p);
		p ++;
		}
	rt_raise(FSERR_FMT, code, buf);
	#endif
}


#if defined(OS_LIMITED)
void	err_missing_rp(void)		{	rt_raise(ERR_MISSING_RP); }
void	err_matdim(void)			{	rt_raise(ERR_MATRIX_DIM); 					}
void	err_syntax(void) 	   		{ 	rt_raise(ERR_SYNTAX);  						}
void	err_syntaxsep(int c)		{ 	rt_raise(ERR_MISSING_SEP, c);  		}
void	err_parm_num(void)			{	rt_raise(ERR_PARCOUNT);			}
#endif

void	err_stackoverflow(void)		{	rt_raise(ERR_STACK_OVERFLOW);						}
void	err_stackunderflow(void)	{	rt_raise(ERR_STACK_UNDERFLOW);					}
void	err_stackmess()				{ 	rt_raise(ERR_STACK);  						}

void	err_arrmis_lp(void)			{	rt_raise(ERR_ARRAY_MISSING_LP); 				}
void	err_arrmis_rp(void)			{	rt_raise(ERR_ARRAY_MISSING_RP); 				}
void	err_arridx(void)			{	rt_raise(ERR_ARRAY_RANGE);			}
void	err_typemismatch(void)		{	rt_raise(ERR_TYPE);						}
void	err_argerr(void)			{	rt_raise(ERR_PARAM);					}

void	err_varisarray(void)		{	rt_raise(EVAL_VAR_IS_ARRAY); 		}
void	err_varisnotarray(void)		{	rt_raise(EVAL_VAR_IS_NOT_ARRAY); }
void	err_vararridx(void)			{	rt_raise(ERR_ARRAY_RANGE); 	}
void	err_varnotnum(void)			{	rt_raise(EVAL_NOT_A_NUM); 				}
void	err_evsyntax(void)			{	rt_raise(EVAL_SYNTAX); 				}
void	err_evtype(void)			{	rt_raise(EVAL_TYPE); 				}
void	err_evargerr(void)			{	rt_raise(EVAL_PARAM); 			}
void	err_unsup(void)				{	rt_raise(ERR_UNSUPPORTED); 						}
void	err_const(void)				{	rt_raise(ERR_CONST);		}
void	err_notavar(void)			{	rt_raise(ERR_NOT_A_VAR);						}

void	err_notarray(void)			{	rt_raise(ERR_NOT_ARR_OR_FUNC); 			}
void	err_out_of_range(void)		{	rt_raise(ERR_RANGE); 						}
void	err_missing_sep(void)		{	rt_raise(ERR_MISSING_SEP_OR_PAR); 	}
void	err_division_by_zero(void)	{	rt_raise(ERR_DIVZERO); 					}
void	err_matop(void)				{	rt_raise(ERR_OPERATOR); 				}
void	err_matsig(void)			{	rt_raise(ERR_MATSIG); 					}
void	err_missing_lp(void)		{	rt_raise(ERR_MISSING_LP); 						}

void	err_parfmt(const char *fmt)	{	rt_raise(ERR_PARFMT, fmt); }
void	err_parm_byref(int n)		{ 	rt_raise(ERR_BYREF, n);  		}

void	err_stridx(void)			{	rt_raise(ERR_STR_RANGE); 		}
void	err_fopen(void)				{	rt_raise(ERR_BAD_FILE_HANDLE);	}
void	err_syntaxanysep(const char *seps)
									{	rt_raise(ERR_SEP_FMT, seps); }
void	err_parsepoly(int idx, int mark)		{	rt_raise(ERR_POLY, idx, mark); }

void	err_bfn_err(long code)
		{	rt_raise(ERR_CRITICAL_MISSING_FUNC, code); }

void	err_gpf(addr_t addr, int bc)
{
	dev_printf(ERR_GPF);
	rt_raise("SEG:CODE[%d]=%02X", addr, bc);
}

void	err_pcode_err(long pcode)
{	rt_raise(ERR_CRITICAL_MISSING_PROC, pcode);	}

void	err_chain_err(const char *file)
									{	rt_raise(ERR_CHAIN_FILE, file);	}

void	err_run_err(const char *file)
									{	rt_raise(ERR_RUN_FILE, file);	}

void	err_invkw(addr_t addr, byte code)
									{		rt_raise(ERR_PARCOUNT_SP, addr, (int) code);	}

/*
*	the DONE message
*/
void	inf_done()
{
	#if defined(_UnixOS)
	if ( !isatty (STDOUT_FILENO) ) 
		fprintf(stdout, "\n* %s *\n", WORD_DONE);
	else {
	#endif

	#if defined(_PalmOS)
	dev_print("\a\n");
	dev_settextcolor(15,0);
	dev_setxy(0,os_graf_my-9);
	dev_printf("\033[91m\033[0m\033[7m * %s * - Press '.' to return\4 \033[80m\033[0m", WORD_DONE);
	#else
	dev_printf("\n\033[0m\033[80m\a\033[7m * %s * \033[0m\n", WORD_DONE);
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
		fprintf(stdout, "\n* %s %d *\n", WORD_BREAK_AT, pline);
	else {
	#endif

	dev_settextcolor(15, 0);
	#if defined(_PalmOS)
	dev_print("\a\n");
	dev_setxy(0,os_graf_my-9);
	dev_printf("\033[91m\033[0m\033[7m * %s %d * - Press '.' to return\4 \033[80m\033[0m", WORD_BREAK_AT, pline);
	#else
	dev_printf("\n\033[0m\033[80m\a\033[7m * %s %d * \033[0m\n", WORD_BREAK_AT, pline);
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


