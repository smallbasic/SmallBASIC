// This file is part of SmallBASIC
//
// Math RTL
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Original headers from matrx042.zip follow: 

/*-----------------------------------------------------------------------------
 *      desc:   matrix mathematics
 *      by:     ko shu pui, patrick
 *      date:   24 nov 91       v0.1b
 *      ref:
 *      [1] Mary L.Boas, "Mathematical Methods in the Physical Sciene,"
 *      John Wiley & Sons, 2nd Ed., 1983. Chap 3.
 *
 *      [2] Kendall E.Atkinson, "An Introduction to Numberical Analysis,"
 *      John Wiley & Sons, 1978.
 *
 *      [3] Alfred V.Aho, John E.Hopcroft, Jeffrey D.Ullman, "The Design
 *      and Analysis of Computer Algorithms," 1974.
 *
 *----------------------------------------------------------------------------*/

#include "common/sys.h"
#include "common/blib_math.h"

// creates a matrix of the given size
var_num_t **mat_create(int row, int col) {
  var_num_t **m = (var_num_t **)malloc(sizeof(var_num_t *) * row);
  int i;
  for (i = 0; i < row; i++) {
    m[i] = (var_num_t *)malloc(sizeof(var_num_t) * col);
  }

  return m;
}

// free an allocated matrix
void mat_free(var_num_t **m, int n) {
  int i;
  for (i = 0; i < n; i++) {
    free(m[i]);
  }
  free(m);
}

// fill the matrix with zero values
void mat_fill(var_num_t **A, int row, int col) {
  int i, j;

  for (i = 0; i < row; i++) {
    for (j = 0; j < col; j++) {
      A[i][j] = 0.0;
    }
  }
}

/*
 *-----------------------------------------------------------------------------
 *       funct:  mat_lu
 *       desct:  in-place LU decomposition with partial pivoting
 *       given:  !! A = square matrix (n x n) !ATTENTION! see commen
 *               P = permutation vector (n x 1)
 *       retrn:  number of permutation performed
 *               -1 means suspected singular matrix
 *       comen:  A will be overwritten to be a LU-composite matrix
 *
 *       note:   the LU decomposed may NOT be equal to the LU of
 *               the orignal matrix a. But equal to the LU of the
 *               rows interchanged matrix.
 *-----------------------------------------------------------------------------
 */
int mat_lu(var_num_t **A, var_num_t **P, int n) {
  int i, j, k, maxi, tmp, p;
  var_num_t c, c1;

  for (p = 0, i = 0; i < n; i++) {
    P[i][0] = i;
  }

  for (k = 0; k < n; k++) {
    /*
     * --- partial pivoting ---
     */
    for (i = k, maxi = k, c = 0.0; i < n; i++) {
      c1 = fabsl(A[(int) P[i][0]][k]);
      if (c1 > c) {
        c = c1;
        maxi = i;
      }
    }

    /*
     * row exchange, update permutation vector
     */
    if (k != maxi) {
      p++;
      tmp = P[k][0];
      P[k][0] = P[maxi][0];
      P[maxi][0] = tmp;
    }

    /*
     * suspected singular matrix
     */
    if (A[(int) P[k][0]][k] == 0.0) {
      return -1;
    }

    for (i = k + 1; i < n; i++) {
      /*
       * --- calculate m(i,j) ---
       */
      A[(int) P[i][0]][k] = A[(int) P[i][0]][k] / A[(int) P[k][0]][k];

      /*
       * --- elimination ---
       */
      for (j = k + 1; j < n; j++) {
        A[(int) P[i][0]][j] -= A[(int) P[i][0]][k] * A[(int) P[k][0]][j];
      }
    }
  }

  return p;
}

/*
 *-----------------------------------------------------------------------------
 *       funct:  mat_backsubs1
 *       desct:  back substitution
 *       given:  A = square matrix A (LU composite)
 *               !! B = column matrix B (attention!, see comen)
 *               !! X = place to put the result of X
 *               P = Permutation vector (after calling mat_lu)
 *               xcol = column of x to put the result
 *       retrn:  column matrix X (of AX = B)
 *       comen:  B will be overwritten
 *-----------------------------------------------------------------------------
 */
var_num_t **mat_backsubs1(var_num_t **A, var_num_t **B, var_num_t **X, var_num_t **P, int xcol, int n) {
  int i, j, k;
  var_num_t sum;

  for (k = 0; k < n; k++) {
    for (i = k + 1; i < n; i++) {
      B[(int) P[i][0]][0] -= A[(int) P[i][0]][k] * B[(int) P[k][0]][0];
    }
  }

  X[n - 1][xcol] = B[(int) P[n - 1][0]][0] / A[(int) P[n - 1][0]][n - 1];
  for (k = n - 2; k >= 0; k--) {
    sum = 0.0;
    for (j = k + 1; j < n; j++) {
      sum += A[(int) P[k][0]][j] * X[j][xcol];
    }
    X[k][xcol] = (B[(int) P[k][0]][0] - sum) / A[(int) P[k][0]][k];
  }

  return X;
}

/*
 *-----------------------------------------------------------------------------
 *      funct:  mat_inv
 *      desct:  find inverse of a matrix
 *      given:  a = square matrix a
 *      retrn:  square matrix Inverse(A)
 *              NULL = fails, singular matrix, or malloc() fails
 *-----------------------------------------------------------------------------
 */
void mat_inverse(var_num_t *a, const int n) {
  var_num_t **A = mat_create(n, n);
  var_num_t **B = mat_create(n, 1);
  var_num_t **C = mat_create(n, n);
  var_num_t **P = mat_create(n, 1);

  int i, j;

  // copy input matrix to working buffer
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      A[i][j] = a[i * n + j];
    }
  }

  // LU-decomposition, also check for singular matrix
  if (mat_lu(A, P, n) != -1) {
    for (i = 0; i < n; i++) {
      mat_fill(B, n, 1);
      B[i][0] = 1.0;
      mat_backsubs1(A, B, C, P, i, n);
    }

    // copy the result in C back to a
    for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) {
        a[i * n + j] = C[i][j];
      }
    }
  }

  // release memory
  mat_free(P, n);
  mat_free(C, n);
  mat_free(B, n);
  mat_free(A, n);
}
