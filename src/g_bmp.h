/*
*	Bitmap manipulation routines
*
*	Nicholas Christopoulos, 13 Feb 2002
*/

#if !defined(_bmp_generic_h)
#define _bmp_generic_h

#include "sys.h"

#define	BMP_AND		2
#define	BMP_OR		4
#define	BMP_XOR		8
#define	BMP_NOT		0x10
#define	BMP_NAND	0x12
#define	BMP_NOR		0x14
#define	BMP_XNOR	0x18

typedef struct {
	char	sign[6];
	int		dx, dy, bpp;
	int		bytespp;
	}	bmp_header_t;

typedef bmp_header_t	bmp_t;

void	bmp_combine(unsigned char *dest, unsigned char *source, int bpl, int lines, int mode)		SEC(BIO);
void	bmp_get(bmp_t *dest, int x1, int y1, int x2, int y2, int bpp, long (*pget)(int x, int y))	SEC(BIO);
void	bmp_put(bmp_t *src, int x1, int y1, void (*pset)(int x, int y, long c)) 					SEC(BIO);
long	bmp_size(int x1, int y1, int x2, int y2, int bpp)											SEC(BIO);
int		bmp_isvalid(bmp_t *bmp)																		SEC(BIO);

#endif

