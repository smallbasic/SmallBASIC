/*
*	formating numbers and strings
*
*	Copyright(c) Nicholas Christopoulos, Nov 2001
*
*	This program is distributed under the terms of the GPL v2.0 or later.
*	Download the GNU Public License (GPL) from www.gnu.org
*/

//#define	_TEST_	// indepented executable

///////////////////////////////////////////////////////////
#if defined(_TEST_)
///////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#define	panic(a)		{ printf("%s\n", (a)); abort(); }
#define	rt_raise(a)		panic(a)
#define	tmp_alloc(x)	malloc(x)
#define	tmp_free(x)		free(x)
#define	dev_print(a)	printf("%s", a)
#define	SEC(x)

// select alogrithm
#define	FMT_USE_i32
//#define	FMT_USE_i64
//#define	FMT_USE_f64
///////////////////////////////////////////////////////////
#else // SB
///////////////////////////////////////////////////////////
#include "sys.h"
#include "str.h"
#include "panic.h"
#include "fmt.h"
#include "device.h"
#include "pproc.h"
#include "messages.h"

//#define	FMT_USE_f64
                  
//#if defined(_UnixOS)
//	#define	FMT_USE_i64
//#else
	#define	FMT_USE_i32
//#endif
///////////////////////////////////////////////////////////
#endif
///////////////////////////////////////////////////////////

//
#if defined(FMT_USE_i32)
// limits for use with 32bit integer algorithm
#define FMT_xMIN	1e-8		// lowest limit to use the exp. format
#define FMT_xMAX	1e+9		// highest limit to use the exp. format
#define FMT_RND		9			// rounding on x digits
#define FMT_xRND	1e+9		// 1 * 10 ^ FMT_RND 
#define FMT_xRND2	1e+8		// 1 * 10 ^ (FMT_RND-1)
#else
// limits for use with 64bit integer or 64bit fp algorithm
#define FMT_RND		14
#define FMT_xRND	1e+14
#define FMT_xRND2	1e+13
#define FMT_xMAX	1e+14
#define FMT_xMIN	1e-8
#endif

// PRINT USING; format-list
#if defined(_PalmOS)
	#define	MAX_FMT_N	32
#else
	#define	MAX_FMT_N	128
#endif
typedef struct {
	char	*fmt;	// the format or a string
	int		type;	// 0 = string, 1 = numeric format, 2 = string format
	} fmt_node_t;

static fmt_node_t	fmt_stack[MAX_FMT_N];		// the list
static int			fmt_count;					// number of elements in the list
static int	   		fmt_cur;					// next format element to be used

/*
*	tables of powers :)
*/
static double nfta_eplus[]=
{ 
1e+8,   1e+16,  1e+24,  1e+32,  1e+40,  1e+48,  1e+56,  1e+64,	// 8
1e+72,  1e+80,  1e+88,  1e+96,  1e+104, 1e+112, 1e+120, 1e+128,	// 16
1e+136, 1e+144, 1e+152, 1e+160, 1e+168, 1e+176, 1e+184, 1e+192,	// 24
1e+200, 1e+208, 1e+216, 1e+224, 1e+232, 1e+240, 1e+248, 1e+256,	// 32
1e+264, 1e+272, 1e+280, 1e+288, 1e+296, 1e+304					// 38
};

static double nfta_eminus[]=
{ 
1e-8,   1e-16,  1e-24,  1e-32, 1e-40,   1e-48,  1e-56,  1e-64,	// 8
1e-72,  1e-80,  1e-88,  1e-96, 1e-104,  1e-112, 1e-120, 1e-128,	// 16
1e-136, 1e-144, 1e-152, 1e-160, 1e-168, 1e-176, 1e-184, 1e-192,	// 24
1e-200, 1e-208, 1e-216, 1e-224, 1e-232, 1e-240, 1e-248, 1e-256,	// 32
1e-264, 1e-272, 1e-280, 1e-288, 1e-296, 1e-304					// 38
};

/*
*	INT(x)
*/
double	fint(double x)	SEC(BMATH);
double	fint(double x)
{
	return (x < 0.0) ? -floor(-x) : floor(x);
}

/*
*	FRAC(x)
*/
double	frac(double x)	SEC(BMATH);
double	frac(double x)
{
	return fabs(fabs(x)-fint(fabs(x)));
}

/*
*	SGN(x)
*/
int		sgn(double x)	SEC(BMATH);
int		sgn(double x)
{
	return (x < 0.0) ? -1 : 1;
}

/*
*	ZSGN(x)
*/
int		zsgn(double x)	SEC(BMATH);
int		zsgn(double x)
{
	return (x < 0.0) ? -1 : ((x > 0.0) ? 1 : 0);
}

/*
*	ROUND(x, digits)
*/
double	fround(double x, int dig)	SEC(BMATH);
double	fround(double x, int dig)
{
	double	m;

	m = floor(pow(10.0, dig));
	if	( x < 0.0 )
		return -floor((-x * m) + .5) / m;
	return floor((x * m) + .5) / m;
}

/*
*	Part of floating point to string (by using integers) algorithm
*	where x any number 2^31 > x >= 0 
*/
void	fptoa(double x, char *dest)
{
	#if defined(FMT_USE_i32)		// 32bit integers
	long	l;

	*dest = '\0';
	l = (long) x;
	#if defined(_PalmOS)
		StrIToA(dest, l);
	#else
		sprintf(dest, "%ld", l);	// or l=atol(dest)
	#endif
	#endif

	#if defined(FMT_USE_i64)		// 64bit integers
	long long	l;

	*dest = '\0';
	l = (long long) x;
	sprintf(dest, "%lld", l);		// or l=atoll(dest)
	#endif

	#if defined(FMT_USE_f64)		// 64bit real
	char	*p;

	sprintf(dest, "%*lf", FMT_RND, x);
	p = strchr(dest, '.');
	if	( p )
		*p = '\0';
	#endif
}

/*
*	remove rightest zeroes from the string
*/
void	rmzeros(char *buf)	SEC(BLIB);
void	rmzeros(char *buf)
{
	char	*p = buf;

	p += (strlen(buf) - 1);
	while ( p > buf )	{
		if	( *p != '0' )
			break;
		*p = '\0';
		p --;
		}
}

/*
*	best float to string (lib)
*
*	This is the real float-to-string routine.
*	It used by the routines:
*		bestfta(double x, char *dest)
*		expfta(double x, char *dest)
*/
void	bestfta_p(double x, char *dest, double minx, double maxx) SEC(BLIB);
void	bestfta_p(double x, char *dest, double minx, double maxx)
{
	double	ipart, fpart, fdif;
	int		sign, i;
	char	*d = dest;
	long	power = 0;
	char	buf[64];

	if	( fabs(x) == 0.0 )	{
		strcpy(dest, "0");
		return;
		}

	// find sign
	sign  = sgn(x);
	if	( sign < 0 )
		*d ++ = '-';
	x = fabs(x);

	if	( x >= 1E308 ) {
		*d = '\0';
		strcat(d, WORD_INF);
		return;
		}
	else if	( x <= 1E-307 ) 	{
		*d = '\0';
		strcat(d, "0");
		return;
		}

	// find power
	if	( x < minx )	{
		for ( i = 37; i >= 0; i -- )	{
			if	( x < nfta_eminus[i] )	{
				x *= nfta_eplus[i];
				power = -((i+1) * 8);
				}
			else
				break;
			}

		while ( x < 1.0 && power > -307 )	{
			x *= 10.0;
			power --;
			}
		}
	else if ( x > maxx )	{
		for ( i = 37; i >= 0; i -- )	{
			if	( x > nfta_eplus[i] )	{
				x /= nfta_eplus[i];
				power = ((i+1) * 8);
				}
			else
				break;
			}

		while ( x >= 10.0 && power < 308 )	{
			x /= 10.0;
			power ++;
			}
		}
	
	// format left part
	ipart = fabs(fint(x));
	fpart = fround(frac(x), FMT_RND) * FMT_xRND;
	if	( fpart >= FMT_xRND )	{	// rounding bug
		ipart = ipart + 1.0;
		if	( ipart >= maxx )	{
			ipart = ipart / 10.0;
			power ++;
			}
		fpart = 0.0;
		}

	fptoa(ipart, buf);
	strcpy(d, buf);
	d += strlen(buf);

	if	( fpart > 0.0 )	{
		// format right part
		*d ++ = '.';
		
		fdif = fpart;
		while ( fdif < FMT_xRND2 )	{
			fdif *= 10;
			*d ++ = '0';
			}

		fptoa(fpart, buf);
		rmzeros(buf);
		strcpy(d, buf);
		d += strlen(buf);
		}

	if	( power )	{
		// add the power
		*d ++ = 'E';
		if	( power > 0 )
			*d ++ = '+';
		fptoa(power, buf);
		strcpy(d, buf);
		d += strlen(buf);
		}
	
	// finish
	*d = '\0';
}

/*
*	best float to string (user)
*/
void	bestfta(double x, char *dest)
{
	bestfta_p(x, dest, FMT_xMIN, FMT_xMAX);
}

/*
* 	float to string (user, E mode)
*/
void	expfta(double x, char *dest)
{
	bestfta_p(x, dest, 1.0, 1.0);
	if	( strchr(dest, 'E') == NULL )
		strcat(dest, "E+0");
}

/*
*	format: map number to format
*
*	dir = direction, 1 = left to right, -1 right to left
*/
void	fmt_nmap(int dir, char *dest, char *fmt, char *src) SEC(BLIB);
void	fmt_nmap(int dir, char *dest, char *fmt, char *src)
{
	char	*p, *d, *s;

	*dest = '\0';
	if	( dir > 0 )	{
		//
		//	left to right
		//
		p = fmt;
		d = dest;
		s = src;
		while ( *p )	{
			switch ( *p )	{
			case '#':
			case '^':
				if	( *s )
					*d ++ = *s ++;
				break;
			case '0':
				if	( *s )
					*d ++ = *s ++;
				else
					*d ++ = '0';
				break;
			default:
				*d ++ = *p;
				}

			p ++;
			}

		*d = '\0';
		}
	else	{
		//
		//	right to left
		//
		p = fmt+(strlen(fmt)-1);
		d = dest+(strlen(fmt)-1);
		*(d+1) = '\0';
		s = src+(strlen(src)-1);
		while ( p >= fmt )	{
			switch ( *p )	{
			case '#':
			case '^':
				if	( s >= src )	
					*d -- = *s --;
				else
					*d -- = ' ';
				break;
			case '0':
				if	( s >= src )	
					*d -- = *s --;
				else
					*d -- = '0';
				break;
			default:
				if	( *p == ',' )	{
					if	( s >= src )	{
						if	( *s == '-' )
							*d -- = *s --;
						else
							*d -- = *p;
						}
					else
						*d -- = ' ';
					}
				else
					*d -- = *p;
				}

			p --;
			}
		}
}

/*
*	format: map number-overflow to format
*/
void	fmt_omap(char *dest, const char *fmt)	SEC(BLIB);
void	fmt_omap(char *dest, const char *fmt)
{
	char	*p = (char *) fmt;
	char	*d = dest;

	while ( *p )	{
		switch ( *p )	{
		case	'#':
		case	'0':
		case	'^':
			*d ++ = '*';
			break;
		default:
			*d ++ = *p;
			}

		p ++;
		}
	*d = '\0';
}

/*
*	format: count digits
*/
int		fmt_cdig(char *fmt)	SEC(BLIB);
int		fmt_cdig(char *fmt)
{
	char	*p = fmt;
	int		count = 0;

	while ( *p )	{
		switch ( *p )	{
		case	'#':
		case	'0':
		case	'^':
			count ++;
			break;
			}

		p ++;
		}

	return count;
}

/*
*	format: format a number
*
*	symbols:
*		# = digit or space
*		0 = digit or zero
*		^ = exponential digit/format
*		. = decimal point
*		, = thousands
*		- = minus for negative
*		+ = sign of number
*/
void	format_num(char *dest, const char *fmt_cnst, double x)
{
	char	*p, *fmt;
	char	left[64], right[64];
	char	lbuf[64], rbuf[64];
	int		dp = 0, lc = 0, sign = 0;
	int		rsz, lsz;

	// backup of format
	fmt = tmp_alloc(strlen(fmt_cnst)+1);
	strcpy(fmt, fmt_cnst);

	// check sign
	if	( strchr(fmt, '-') || strchr(fmt, '+') )	{
		sign = 1;
		if	( x < 0.0 )	{
			sign = -1;
			x = -x;
			}
		}

	if	( strchr(fmt_cnst, '^') )	{
		//
		//	E format
		//

		lc = fmt_cdig(fmt);
		if	( lc < 4 )	{
			fmt_omap(dest, fmt);
			tmp_free(fmt);
			return;
			}

		// convert
		expfta(x, dest);

		// format
		p = strchr(dest, 'E');
		if	( p )	{
			*p = '\0';
			strcpy(left, dest);
			strcpy(right, p+1);
			lsz = strlen(left);
			rsz = strlen(right)+1;

			if	( lc < rsz+1 )	{
				fmt_omap(dest, fmt);
				tmp_free(fmt);
				return;
				}
			
			if	( lc < lsz + rsz + 1 )	
				left[lc-rsz] = '\0';

			strcpy(lbuf, left);
			strcat(lbuf, "E");
			strcat(lbuf, right);
			fmt_nmap(-1, dest, fmt, lbuf);
			}
		else	{
			strcpy(left, dest);
			fmt_nmap(-1, dest, fmt, left);
			}
		}
	else	{
		//
		//	normal format
		//

		// rounding
		p = strchr(fmt, '.');
		if	( p )	
			x = fround(x, fmt_cdig(p+1));
		else
			x = fround(x, 0);

		// convert
		bestfta(x, dest);
		if	( strchr(dest, 'E') )	{
			fmt_omap(dest, fmt);
			tmp_free(fmt);
			return;
			}

		// left & right parts
		left[0] = right[0] = '\0';
		p = strchr(dest, '.');
		if	( p )	{
			*p = '\0';
			strcpy(right, p+1);
			}
		strcpy(left, dest);

		// map format
		rbuf[0] = lbuf[0] = '\0';
		p = strchr(fmt, '.');
		if	( p )	{
			dp = 1;
			*p = '\0';
			fmt_nmap(1, rbuf, p+1, right);
			}

		lc = fmt_cdig(fmt);
		if	( lc < strlen(left) )	{
			fmt_omap(dest, fmt_cnst);
			tmp_free(fmt);
			return;
			}
		fmt_nmap(-1, lbuf, fmt, left);

		strcpy(dest, lbuf);
		if	( dp )	{
			strcat(dest, ".");
			strcat(dest, rbuf);
			}
		}

	// sign in format
	if	( sign )	{		// 24/6 Snoopy42 modifications
		char	*e;	
	
		e = strchr(dest, 'E');
		if ( e )   {  // special treatment for E format 
			p = strchr(dest, '+'); 
			if   ( p && p < e ) // the sign bust be before the E    
				*p = (sign > 0) ? '+' : '-';    

			p = strchr(dest, '-'); 
			if   ( p && p < e )    
				*p = (sign > 0) ? ' ' : '-'; 
      		}
		else   { // no E format		
			p = strchr(dest, '+');
			if	( p )	
				*p = (sign > 0) ? '+' : '-';

			p = strchr(dest, '-');
			if	( p )	
				*p = (sign > 0) ? ' ' : '-';
			}
		}

	// cleanup
	tmp_free(fmt);
}

/*
*	format: format a string
*
*	symbols:
*		&	the whole string
*		!	the first char
*		\\	segment 
*/
void	format_str(char *dest, const char *fmt_cnst, const char *str)
{
	char	*p, *d, *ss = NULL;
	int		ps = 0, pe = 0, l, srclen;
	int		count, lc;

	if	( strchr(fmt_cnst, '&') )	{
		strcpy(dest, str);
		return;
		}
	if	( strchr(fmt_cnst, '!') )	{
		dest[0] = str[0];
		dest[1] = '\0';
		return;
		}

	// segment
	l = strlen(fmt_cnst);
	srclen = strlen(str);
	p = (char *) fmt_cnst;
	lc = 0;
	count = 0;
	while ( *p )	{
		if	( *p == '\\' && lc != '_' )	{
			if	( count == 0 )	{
				ss = p;
				ps = (int) (p - fmt_cnst);
				count ++;
				}
			else	{
				pe = p - fmt_cnst;
				count ++;
				break;
				}
			}
		else if ( count )
			count ++;

		lc = *p;
		p ++;
		}
	
	memset(dest, ' ', l-1);
	dest[l] = '\0';
	d = dest;
	if	( ps )	{
		memcpy(d, fmt_cnst, ps);
		d += ps;
		} 

	/*
	*	convert
	*/
	if	( ss )	{
		int		i, j;

		for ( i = j = 0; i < count; i ++ )	{
			switch ( ss[i] )	{
			case	'\\':
			case	' ':
				if	( j < srclen ) {
					d[i] = str[j];
					j ++;
					}
				else
					d[i] = ' ';
				break;
			default:
				d[i] = ss[i];
				}
			}
		}

	//
	d += count;
	*d = '\0';
	if	( *(fmt_cnst+pe+1) != '\0' )	
		strcat(dest, fmt_cnst+pe+1);
}

/*
*	get numeric format
*/
char	*fmt_getnumfmt(char *dest, char *source)	SEC(BLIB);
char	*fmt_getnumfmt(char *dest, char *source)
{
	int		dp = 0, sign = 0, exitf = 0;
	char	*p = source;
	char	*d = dest;

	while ( *p )	{
		switch ( *p )	{
		case	'^':
		case	'#':
		case	'0':
		case	',':
			*d ++ = *p;
			break;
		case	'-':
		case	'+':
			sign ++;
			if	( sign > 1 )	
				exitf = 1;
			else
				*d ++ = *p;
			break;
		case	'.':
			dp ++;
			if	( dp > 1 )	
				exitf = 1;
			else
				*d ++ = *p;
			break;
		default:
			exitf = 1;
			}

		if	( exitf )
			break;

		p ++;
		}

	*d = '\0';
	return p;
}

/*
*	get string format
*/
char	*fmt_getstrfmt(char *dest, char *source)	SEC(BLIB);
char	*fmt_getstrfmt(char *dest, char *source)
{
	char	*p = source;
	char	*d = dest;

	if	( source[0] == '&' || source[0] == '!' )	{
		*d ++ = *source; *d ++ = '\0';
		return p+1;
		}
	
	while ( *p )	{
		*d ++ = *p ++;
		if	( *p == '\\' )	{
			*d ++ = *p ++;
			break;
			}
		}

	*d = '\0';
	return p;
}

/*
*	add format node
*/
void	fmt_addfmt(const char *fmt, int type) SEC(BLIB);
void	fmt_addfmt(const char *fmt, int type)
{
	fmt_node_t	*node;

	node = &fmt_stack[fmt_count];
	fmt_count ++;
	if	( fmt_count >= MAX_FMT_N )	
		panic("Maximum format-node reached");
	node->fmt = tmp_alloc(strlen(fmt)+1);
	strcpy(node->fmt, fmt);
	node->type = type;
}

/*
*	cleanup format-list
*/
void	free_format()
{
	int		i;
	fmt_node_t	*node;

	for ( i = 0; i < fmt_count; i ++ )	{
		node = &fmt_stack[i];
		tmp_free(node->fmt);
		}

	fmt_count = fmt_cur = 0;
}

/*
*	The final format - create the format-list 
*	(that list it will be used later by fmt_printN and fmt_printS)
*
*	'_' the next character is not belongs to format (simple string)
*/
void	build_format(const char *fmt_cnst)
{
	char	*fmt;
	char	*p;
	int		nc;
	#if	defined(OS_LIMITED)
	char	buf[128], *b;
	#else
	char	buf[1024], *b;
	#endif

	free_format();
		
	// backup of format
	fmt = tmp_alloc(strlen(fmt_cnst)+1);
	strcpy(fmt, fmt_cnst);

	p = fmt;
	b = buf;
	nc = 0;
	while ( *p )	{
		switch ( *p )	{
		case	'_':
			// store prev. buf
			*b = '\0';
			if	( strlen(buf) )
				fmt_addfmt(buf, 0);
			
			// store the new
			buf[0] = *(p+1);
			buf[1] = '\0';
			fmt_addfmt(buf, 0);
			b = buf;
			p ++;
			break;
		case	'-':
		case	'+':
		case	'^':
		case	'0':
		case	'#':
			// store prev. buf
			*b = '\0';
			if	( strlen(buf) )
				fmt_addfmt(buf, 0);

			// get num-fmt
			p = fmt_getnumfmt(buf, p);
			fmt_addfmt(buf, 1);
			b = buf;
			nc = 1;
			break;
		case	'&':
		case	'!':
		case	'\\':
			// store prev. buf
			*b = '\0';
			if	( strlen(buf) )
				fmt_addfmt(buf, 0);

			// get str-fmt
			p = fmt_getstrfmt(buf, p);
			fmt_addfmt(buf, 2);
			b = buf;
			nc = 1;
			break;
		default:
			*b ++ = *p;
			}

		if	( *p )	{
			if	( nc )	// do not advance
				nc = 0;
			else
				p ++;
			}
		}

	// store prev. buf
	*b = '\0';
	if	( strlen(buf) )	
		fmt_addfmt(buf, 0);

	// cleanup
	tmp_free(fmt);
}

/*
*	print simple strings (parts of format)
*/
#if defined(_TEST)
void	fmt_printL()
#else
void	fmt_printL(int output, int handle)	SEC(BLIB);
void	fmt_printL(int output, int handle)
#endif
{
	fmt_node_t	*node;
	
	if	( fmt_count == 0 )	
		return;
	else	{
		do	{
			node = &fmt_stack[fmt_cur];
			if	( node->type == 0 )	{
				#if defined(_TEST)
				dev_print(node->fmt);
				#else
				pv_write(node->fmt, output, handle);
				#endif
				fmt_cur ++;
				if ( fmt_cur >= fmt_count )
					fmt_cur = 0;
				}
			} while ( node->type == 0 && fmt_cur != 0 );
		}
}

/*
*	print formated number
*/
#if defined(_TEST)
void	fmt_printN(double x)
#else
void	fmt_printN(double x, int output, int handle)
#endif
{
	fmt_node_t	*node;
	char		buf[64];

	if	( fmt_count == 0 )	{
		rt_raise(ERR_FORMAT_INVALID_FORMAT);
		}
	else	{
		#if defined(_TEST)
		fmt_printL();
		#else
		fmt_printL(output, handle);
		#endif
		node = &fmt_stack[fmt_cur];
		fmt_cur ++;
		if ( fmt_cur >= fmt_count )
			fmt_cur = 0;
		if	( node->type == 1 )	{
			format_num(buf, node->fmt, x);
			#if defined(_TEST)
			dev_print(buf);
			#else
			pv_write(buf, output, handle);
			#endif
			if ( fmt_cur != 0 )		{
				#if defined(_TEST)
				fmt_printL();
				#else
				fmt_printL(output, handle);
				#endif
				}
			}
		else	{
			rt_raise(ERR_FORMAT_INVALID_FORMAT);
			}
		}
}

/*
*	print formated string
*/
#if defined(_TEST)
void	fmt_printS(const char *str)
#else
void	fmt_printS(const char *str, int output, int handle)
#endif
{
	fmt_node_t	*node;
	#if defined(_PalmOS)
	char		buf[64];
	#else
	char		buf[1024];
	#endif

	if	( fmt_count == 0 )	{
		rt_raise(ERR_FORMAT_INVALID_FORMAT);
		}
	else	{
		#if defined(_TEST)
		fmt_printL();
		#else
		fmt_printL(output, handle);
		#endif
		node = &fmt_stack[fmt_cur];
		fmt_cur ++;
		if ( fmt_cur >= fmt_count )
			fmt_cur = 0;
		if	( node->type == 2 )	{
			format_str(buf, node->fmt, str);
			#if defined(_TEST)
			dev_print(buf);
			#else
			pv_write(buf, output, handle);
			#endif
			if ( fmt_cur != 0 )		{
				#if defined(_TEST)
				fmt_printL();
				#else
				fmt_printL(output, handle);
				#endif
				}
			}
		else	{
			rt_raise(ERR_FORMAT_INVALID_FORMAT);
			}
		}
}

#if defined(_TEST_)
/*
*/
int main()
{
	char	buf[1024];
	double	x;

	build_format("Total --#,##0.00 of / / goods");	// PRINT USING - build format

	do	{
		// input
		printf("\nx? ");	gets(buf);
		if	( buf[0] == 'q' )
			break;
		sscanf(buf, "%lf", &x);

		// numbers
		bestfta(x, buf);
		printf("bestfta(): [%s]\n", buf);

		format_num(buf, "#,###,##0.00", x);
		printf("fmt_num(): [%s]\n", buf);

		format_num(buf, "^^^^^^", x);
		printf("fmt_num(): [%s]\n", buf);

		// strings
		format_str(buf, "12/ /34", "Hello, World");
		printf("fmt_str(): [%s]\n", buf);

		format_str(buf, "/ /", "Hello, World");
		printf("fmt_str(): [%s]\n", buf);

		format_str(buf, "12 !34", "Hello, World");
		printf("fmt_str(): [%s]\n", buf);

		format_str(buf, "123 & 4", "Hello, World");
		printf("fmt_str(): [%s]\n", buf);

		// PRINT USING
		fmt_printN(x);
		fmt_printS("ABCDEF");

		} while ( 1 );

	free_format();	// PRINT USING - free format-data
}
#endif
