/**
 * @file blib_math.h
 * SmallBASIC - Math RTL
 *
 * Nicholas Christopoulos
 */

/**
 * @defgroup math Mathematics
 */

#if !defined(_blib_math_h)
#define _blib_math_h

#include "sys.h"

#define SEGLEN(Ax,Ay,Bx,By)     line_length(Ax,Ay,Bx,By)
#define PTSIGN(Ax,Ay,Bx,By,Qx,Qy) (ZSGN((Qx) * ((Ay) - (By)) + (Qy) * ((Bx) - (Ax)) + (Ax) * (By) - (Ay) * (Bx))) /**< sign of a point(Q) from a line-segment(A->B) @ingroup math */

/**
 * @ingroup math
 *
 * solve linear equations. Gauss-Jordan method.
 *
 * the result will stored on 'b'
 *
 * @param a is the first table
 * @param b is the second table
 * @param n is the number of the rows
 * @param toler is the smallest acceptable number
 */
void mat_gauss_jordan(double *a, double *b, int n, double toler) SEC(BMATH2);

/**
 * @ingroup math
 *
 * converts the matrix A to its inverse.
 *
 * @param a is the matrix
 * @param n is the number of rows/cols
 */
void mat_inverse(double *a, int n) SEC(BMATH2);

void mat_det2(double t, int m, int k, double *a, int *done, double *v, int n,
              double toler) SEC(BMATH2);

/**
 * @ingroup math
 *
 * determinant of A
 *
 * @param a is the matrix
 * @param n is the rows/cols of A
 * @param toler is the smallest acceptable number
 * @return the determinant of A
 */
double mat_determ(double *a, int n, double toler) SEC(BMATH2);

/**
 * @ingroup math
 * todo: statmeandev
 */
double statmeandev(double *e, int count) SEC(BMATH2);

/**
 * @ingroup math
 * todo: statspreads
 */
double statspreads(double *e, int count) SEC(BMATH2);

/**
 * @ingroup math
 * todo: statspreadp
 */
double statspreadp(double *e, int count) SEC(BMATH2);

#endif
