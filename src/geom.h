/**
*	@file geom.h
*
*	SmallBASIC, extra geometry algorithms
*
*	This program is distributed under the terms of the GPL v2.0 or later
*	Download the GNU Public License (GPL) from www.gnu.org
*
*	History:
*	ndc - 09/03/2003 - created
*/

/**
*	@defgroup math Mathematics
*/

#if !defined(_blib_geom_h)
#define _blib_geom_h

#include "sys.h"
#include "pproc.h"

/**
*	@ingroup math
*
*	returns the length of a line-segment
*
*	@param Ax the point A
*	@param Ay the point A
*	@param Ax the point B
*	@param Ay the point B
*	@return the length of a line-segment
*/
double geo_seglen(double Ax, double Ay, double Bx, double By)				SEC(BMATH2);

/**
*	@ingroup math
*
*	distance of point C from line (A,B)
*/
double geo_distfromline(double Ax, double Ay, double Bx, double By, double Cx, double Cy) SEC(BMATH2);

/**
*	@ingroup math
*
*	distance of point C from line segment (A->B)
*/
double geo_distfromseg(double Ax, double Ay, double Bx, double By, double Cx, double Cy) SEC(BMATH2);

/**
*	@ingroup math
*
*	returns the angle of two vectors
*
*	@param type kwSEGCOS or kwSEGSIN, return the angle's cosine or the sine
*	@param Adx the first vector (dx)
*	@param Ady the first vector (dy)
*	@param Bdx the second vector (dx)
*	@param Bdy the second vector (dy)
*	@return the cosine or the sine of two vectors
*/
double	geo_segangle(int type, double Adx, double Ady, double Bdx, double Bdy) SEC(BMATH2);

/**
*	@ingroup math
*
*	Calculates the centroid (xCentroid, yCentroid) and area
*	of a polygon, given its vertices (x[0], y[0]) ... (x[n-1], y[n-1]). 
*	It is assumed that the contour is closed, i.e., that the vertex following
*	(x[n-1], y[n-1]) is (x[0], y[0]).  The algebraic sign of the area is
*	positive for counterclockwise ordering of vertices in x-y plane;
*	otherwise negative.
*
*	Returned values:  0 for normal execution;  1 if the polygon is
*	degenerate (number of vertices < 3);  and 2 if area = 0 (and the
*	centroid is undefined).
*/
int		geo_polycentroid(pt_t *poly, int n, double *xCentroid, double *yCentroid, double *area) SEC(BMATH2);

#endif


