// This file is part of SmallBASIC
//
// Math RTL
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

/**
 * @defgroup math Mathematics
 */

#if !defined(_blib_math_h)
#define _blib_math_h

#include "common/sys.h"

var_num_t fint(var_num_t x);
var_num_t frac(var_num_t x);
int sgn(var_num_t x);
var_num_t fround(var_num_t x, int dig);

#define PTSIGN(Ax,Ay,Bx,By,Qx,Qy) (ZSGN((Qx) * ((Ay) - (By)) + (Qy) * \
     ((Bx) - (Ax)) + (Ax) * (By) - (Ay) * (Bx))) 
/**< sign of a point(Q) from a line-segment(A->B) @ingroup math */

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
void mat_gauss_jordan(var_num_t *a, var_num_t *b, int n, double toler);

/**
 * @ingroup math
 *
 * converts the matrix A to its inverse.
 *
 * @param a is the matrix
 * @param n is the number of rows/cols
 */
void mat_inverse(var_num_t *a, int n);

void mat_det2(var_num_t t, int m, int k, var_num_t *a, int *done, var_num_t *v, int n, double toler)
   ;

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
var_num_t mat_determ(var_num_t *a, int n, double toler);

/**
 * @ingroup math
 * todo: statmeandev
 */
var_num_t statmeandev(var_num_t *e, int count);

/**
 * @ingroup math
 * todo: statspreads
 */
var_num_t statspreads(var_num_t *e, int count);

/**
 * @ingroup math
 * todo: statspreadp
 */
var_num_t statspreadp(var_num_t *e, int count);

#endif
