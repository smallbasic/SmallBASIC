/*
*	bc module. Bytecode manipulation routines (bytecode segments API)
*
*	2001/02/23, Nicholas Christopoulos
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*/

#include "bc.h"
#include "smbas.h"
#if defined(_UnixOS)
#include <assert.h>
#endif

void	bc_add2c(bc_t *bc, byte code, byte v)	SEC(BCSCAN);
#define	bc_add2(a,b,c)	bc_add2c((a),(b),(c))
void	bc_add2i(bc_t *bc, byte code, word p1)	SEC(BCSCAN);
void	bc_add2l(bc_t *bc, byte code, long p1)	SEC(BCSCAN);
void	bc_add2d(bc_t *bc, byte code, double p1) SEC(BCSCAN);
void	bc_add2s(bc_t *bc, byte code, const char *p1) SEC(BCSCAN);

/*
*	Create a bytecode segment
*/
void	bc_create(bc_t *bc)
{
	bc->mem_h = mem_alloc(BC_ALLOC_INCR);
	bc->ptr   = mem_lock(bc->mem_h);
	bc->size  = BC_ALLOC_INCR;
	bc->count = 0;
	bc->cp = 0;
}

/*
*	Destroy a bytecode segment
*/
void	bc_destroy(bc_t *bc)
{
	mem_unlock(bc->mem_h);
	mem_free(bc->mem_h);
	bc->ptr = NULL;
	bc->size = 0;
	bc->count = 0;
	bc->cp = 0;
}

/*
*	Resize a bytecode segment
*/
void	bc_resize(bc_t *bc, dword new_size)
{
	mem_unlock(bc->mem_h);
	bc->mem_h = mem_realloc(bc->mem_h, new_size);
	bc->size = new_size;
	bc->ptr = mem_lock(bc->mem_h);
}

/*
*	add one command 
*/
void	bc_add1(bc_t *bc, byte code)
{
	if	( bc->count >= (bc->size - 2) )
		bc_resize(bc, bc->size + BC_ALLOC_INCR);

	bc->ptr[bc->count] = code;
	bc->count ++;
}

/*
*	change one command 
*/
void	bc_store1(bc_t *bc, addr_t offset, byte code)
{
	bc->ptr[offset] = code;
}

/*
*	add one command & one byte
*/
void	bc_add2c(bc_t *bc, byte code, byte v)
{
	if	( bc->count >= bc->size - 3 )
		bc_resize(bc, bc->size + BC_ALLOC_INCR);

	bc->ptr[bc->count] = code;
	bc->count ++;
	bc->ptr[bc->count] = v;
	bc->count ++;
}

/*
*	add one word
*/
void	bc_add_word(bc_t *bc, word p1)
{
	if	( bc->count >= bc->size - 4 )
		bc_resize(bc, bc->size + BC_ALLOC_INCR);

	memcpy(bc->ptr+bc->count, &p1, 2);
	bc->count += 2;
}

/*
*	add one dword
*/
void	bc_add_dword(bc_t *bc, dword p1)
{
	if	( bc->count >= bc->size - 4 )
		bc_resize(bc, bc->size + BC_ALLOC_INCR);

	memcpy(bc->ptr+bc->count, &p1, 4);
	bc->count += 4;
}

/*
*	add one command and one word
*/
void	bc_add2i(bc_t *bc, byte code, word p1)
{
	if	( bc->count >= bc->size - 4 )
		bc_resize(bc, bc->size + BC_ALLOC_INCR);

	bc->ptr[bc->count] = code;
	bc->count ++;
	memcpy(bc->ptr+bc->count, &p1, 2);
	bc->count += 2;
}

/*
*	add one command and one long int
*/
void	bc_add2l(bc_t *bc, byte code, long p1)
{
	if	( bc->count >= bc->size - 8 )
		bc_resize(bc, bc->size + BC_ALLOC_INCR);

	bc->ptr[bc->count] = code;
	bc->count ++;
	memcpy(bc->ptr+bc->count, &p1, 4);
	bc->count += 4;
}

/*
*	add buildin function call
*/
void	bc_add_fcode(bc_t *bc, long idx)
{
#if defined(OS_ADDR16)
	bc_add2i(bc, kwTYPE_CALLF, idx);
#else
	bc_add2l(bc, kwTYPE_CALLF, idx);
#endif
}

/*
*	add buildin procedure call
*/
void	bc_add_pcode(bc_t *bc, long idx)
{
#if defined(OS_ADDR16)
	bc_add2i(bc, kwTYPE_CALLP, idx);
#else
	bc_add2l(bc, kwTYPE_CALLP, idx);
#endif
}

/*
*	add an external function-call
*/
void	bc_add_extfcode(bc_t *bc, int lib, long idx)
{
	bc_add_code(bc, kwTYPE_CALLEXTF);
#if defined(OS_ADDR16)
	bc_add_word(bc, lib);
	bc_add_word(bc, idx);
#else
	bc_add_dword(bc, lib);
	bc_add_dword(bc, idx);
#endif
}

/*
*	add an external procedure-call
*/
void	bc_add_extpcode(bc_t *bc, int lib, long idx)
{
	bc_add_code(bc, kwTYPE_CALLEXTP);
#if defined(OS_ADDR16)
	bc_add_word(bc, lib);
	bc_add_word(bc, idx);
#else
	bc_add_dword(bc, lib);
	bc_add_dword(bc, idx);
#endif
}

/*
*	add an address
*/
void	bc_add_addr(bc_t *bc, addr_t idx)
{
	if	( bc->count >= bc->size - 4 )
		bc_resize(bc, bc->size + BC_ALLOC_INCR);

	#if defined(OS_ADDR16)
	memcpy(bc->ptr+bc->count, &idx, 2);
	bc->count += 2;
	#else
	memcpy(bc->ptr+bc->count, &idx, 4);
	bc->count += 4;
	#endif
}

/*
*	add a control code
*
*	Control codes are followed by 2 addr_t elements (jump on true, jump on false)
*/
void	bc_add_ctrl(bc_t *bc, code_t code, addr_t true_ip, addr_t false_ip)
{
	bc_add1(bc, code);
	bc_add_addr(bc, true_ip);
	bc_add_addr(bc, false_ip);
}

/*
*	add an integer
*/
void	bc_add_cint(bc_t *bc, long v)
{
	bc_add2l(bc, kwTYPE_INT, v);
}

/*
*	add an real
*/
void	bc_add_creal(bc_t *bc, double v)
{
	bc_add2d(bc, kwTYPE_NUM, v);
}

/*
*	add one command and one double
*/
void	bc_add2d(bc_t *bc, byte code, double p1)
{
	if	( bc->count >= bc->size - 16 )
		bc_resize(bc, bc->size + BC_ALLOC_INCR);

	bc->ptr[bc->count] = code;
	bc->count ++;
	memcpy(bc->ptr+bc->count, &p1, 8);
	bc->count += 8;
}

/*
*	add one command and one string (see: bc_store_string)
*/
void	bc_add2s(bc_t *bc, byte code, const char *p1)
{
	word	l;

	l = strlen(p1);
	if	( l > BC_MAX_STORE_SIZE )
		sc_raise("STRING TOO BIG");
	else	{
		bc_add_code(bc, code);
		#if defined(OS_ADDR16)
		bc_add_word(bc, l);
		#else
		bc_add_dword(bc, l);
		#endif

		if	( bc->count >= bc->size - l )
			bc_resize(bc, bc->size + BC_ALLOC_INCR);

		memcpy(bc->ptr+bc->count, p1, l);
		bc->count += l;
		}
}

/*
*	adds a string.
*	returns a pointer of src to the next "element"
*/
char*	bc_store_string(bc_t *bc, char *src)
{
	char	*p = src, *np;
	int		l;

	p ++;	// == '\"'
	while ( *p )	{
		if	( *p == '\"' )	{
			l = p - src;
			np = tmp_alloc(l+1);
			strncpy(np, src+1, l);
			np[l-1] = '\0';

			if ( opt_cstr )	{
				// c-style special char syntax
				char *cstr;
				cstr = cstrdup(np);
				tmp_free(np);
				bc_add2s(bc, kwTYPE_STR, cstr);
				tmp_free(cstr);
				}
			else	{
				// normal
				bc_add2s(bc, kwTYPE_STR, np);
				tmp_free(np);
				}

			p ++;
			return p;
			}
		p ++;
		}

	return p;
}

/*
*	adds a string.
*	returns a pointer of src to the next "element"
*/
char*	bc_store_macro(bc_t *bc, char *src)
{
	char	*p = src, *np;
	int		l;

	p ++;	// == `
	while ( *p )	{
		if	( *p == '`' )	{
			l = p - src;
			np = tmp_alloc(l+1);
			strncpy(np, src+1, l);
			np[l-1] = '\0';
			bc_add2s(bc, kwTYPE_STR, np);
			tmp_free(np);

			p ++;
			return p;
			}
		p ++;
		}

	return p;
}

/*
*	adds an EOC mark at the current position
*/
void	bc_eoc(bc_t *bc)
{
//	if	( bc->count )	{
//		if	( bc->ptr[bc->count-1] != kwTYPE_EOC )
			bc_add1(bc, kwTYPE_EOC);
//		}
}

/*
*	appends the src to dst
*/
void	bc_append(bc_t *dst, bc_t *src)
{
	bc_resize(dst, dst->count + src->count + 4);
	memcpy(dst->ptr + dst->count, src->ptr, src->count);
	dst->count += src->count;
}

/*
*	appends n bytes from src to dst
*/
void	bc_add_n(bc_t *dst, byte *src, dword n)
{
	if	( dst->count >= dst->size - n )
		bc_resize(dst, dst->size + n);
	memcpy(dst->ptr+dst->count, src, n);
	dst->count += n;
}



