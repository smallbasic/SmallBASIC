// $Id$
// This file is part of SmallBASIC
//
// SmallBASIC LIBRARY - extra geometry algorithms
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "geom.h"
#include "kw.h"

/**
 * length of line segment A-B
 */
double geo_seglen(double Ax, double Ay, double Bx, double By)
{
  double dx = Bx - Ax;
  double dy = By - Ay;

  return sqrt(dx * dx + dy * dy);
}

/**
 * distance of point C from line (A,B)
 */
double geo_distfromline(double Ax, double Ay, double Bx, double By, double Cx,
                        double Cy)
{
  double L, s;
//      double  r;
//      double  Px, Py; // projection of C on AB

  L = geo_seglen(Ax, Ay, Bx, By);

  if (L == 0.0)                 // A,B are the same point, TODO: tolerance
    return geo_seglen(Ax, Ay, Cx, Cy);

//      r = ((Ay-Cy) * (Ay-By) - (Ax-Cx) * (Bx-Ax)) / (L * L);
//      Px = Ax + r * (Bx - Ax);
//      Py = Ay + r * (By - Ay);

  s = ((Ay - Cy) * (Bx - Ax) - (Ax - Cx) * (By - Ay)) / (L * L);
  return s * L;
}

/**
 * distance of point C from line segment (A->B)
 */
double geo_distfromseg(double Ax, double Ay, double Bx, double By, double Cx,
                       double Cy)
{
//      double  Px, Py; // projection of C on AB
  double L, r, s;
  double ca, cb;

  L = geo_seglen(Ax, Ay, Bx, By);

  if (L == 0.0)                 // A,B are the same point, TODO: tolerance
    return geo_seglen(Ax, Ay, Cx, Cy);

  r = ((Ay - Cy) * (Ay - By) - (Ax - Cx) * (Bx - Ax)) / (L * L);

  if (r >= 0.0 && r <= 1.0) {   // TODO: tolerance
//              Px = Ax + r * (Bx - Ax);
//              Py = Ay + r * (By - Ay);

    s = ((Ay - Cy) * (Bx - Ax) - (Ax - Cx) * (By - Ay)) / (L * L);
    return s * L;
  }

  // else the P is out of A-B
  ca = geo_seglen(Ax, Ay, Cx, Cy);
  cb = geo_seglen(Bx, By, Cx, Cy);
  return (ca < cb) ? ca : cb;
}

/*
 * COS/SIN of two line segments
 */
double geo_segangle(int type, double Adx, double Ady, double Bdx, double Bdy)
{
  double la, lb;
  double UAdx, UAdy, UBdx, UBdy;

  la = sqrt(Adx * Adx + Ady * Ady);
  lb = sqrt(Bdx * Bdx + Bdy * Bdy);

  if (la != 0 && lb != 0) {
    UAdx = Adx / la;
    UAdy = Ady / la;
    UBdx = Bdx / lb;
    UBdy = Bdy / lb;
    if (type == kwSEGCOS)
      return UAdx * UBdx + UAdy * UBdy;
    else
      return UAdy * UBdx + UAdx * UBdy;
  }

  return 0;                     // Ενα από τα διανύσματα έχει μήκος 0.
}

/*********************************************************************
ANSI C code from the article "Centroid of a Polygon"
                                  by Gerard Bashein and Paul R. Detmer

polyCentroid: Calculates the centroid (xCentroid, yCentroid) and area
of a polygon, given its vertices (x[0], y[0]) ... (x[n-1], y[n-1]). It
is assumed that the contour is closed, i.e., that the vertex following
(x[n-1], y[n-1]) is (x[0], y[0]).  The algebraic sign of the area is
positive for counterclockwise ordering of vertices in x-y plane;
otherwise negative.

Returned values:  0 for normal execution;  1 if the polygon is
degenerate (number of vertices < 3);  and 2 if area = 0 (and the
centroid is undefined).
**********************************************************************/
int geo_polycentroid(pt_t * poly, int n, var_num_t *xCentroid, var_num_t *yCentroid,
                     var_num_t *area)
{
  int i, j;
  var_num_t ai, atmp = 0, xtmp = 0, ytmp = 0;

  if (n < 3) {
    return 1;
  }

  for (i = n - 1, j = 0; j < n; i = j, j++) {
    ai = poly[i].x * poly[j].y - poly[j].x * poly[i].y;
    atmp += ai;
    xtmp += (poly[j].x + poly[i].x) * ai;
    ytmp += (poly[j].y + poly[i].y) * ai;
  }

  *area = atmp / 2;
  if (atmp != 0) {
    *xCentroid = xtmp / (3 * atmp);
    *yCentroid = ytmp / (3 * atmp);
    return 0;
  }

  return 2;
}
