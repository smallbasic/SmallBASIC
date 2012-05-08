// This file is part of SmallBASIC
//
// PolyLineFill
// See: "Zen of graphics programming", Chapter 24. L24-1.C.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/device.h"

struct HLine {
  int XStart; /* X coordinate of leftmost pixel in line */
  int XEnd; /* X coordinate of rightmost pixel in line */
};

struct HLineList {
  int Length; /* # of horizontal lines */
  int YStart; /* Y coordinate of topmost line */
  struct HLine *HLinePtr; /* pointer to list of horz lines */
};

struct EdgeState {
  struct EdgeState *NextEdge;
#if defined(OS_LIMITED)
  int16 X, StartY;
  int16 WholePixelXMove;
  int16 XDirection;
  int16 ErrorTerm;
  int16 ErrorTermAdjUp;
  int16 ErrorTermAdjDown;
  int16 Count;
#else
  int X, StartY;
  int WholePixelXMove;
  int XDirection;
  int ErrorTerm;
  int ErrorTermAdjUp;
  int ErrorTermAdjDown;
  int Count;
#endif
};
// 18bytes

void pf_build_GET(ipt_t *, int, struct EdgeState *) SEC(BIO);
void pf_move_xsorted_AET(int) SEC(BIO);
void pf_scan_out_AET(int) SEC(BIO);
void pf_advance_AET(void) SEC(BIO);
void pf_xsort_AET(void) SEC(BIO);

/* 
 *	Pointers to global edge table (GET) and active edge table (AET) 
 */
static struct EdgeState *GETPtr;
static struct EdgeState *AETPtr;

/*
 *	FillPoly
 *
 *	XOffset, YOffset	Coordinates to draw the polygon.
 *	*VertexList		The array of the points.
 *	ptNum			The number of points.
 *	Color			The color.
 */
void dev_pfill(ipt_t * pts, int ptNum) {
  struct EdgeState *EdgeTableBuffer;
  int CurrentY;

  /*
   *      It takes a minimum of 3 vertices to cause any pixels to be
   *      drawn; reject polygons that are guaranteed to be invisible 
   */
  if (ptNum < 3)
    return;

  EdgeTableBuffer = (struct EdgeState *) tmp_alloc(sizeof(struct EdgeState) * (ptNum + 1));

  /*
   * Build the global edge table. 
   */

  pf_build_GET(pts, ptNum, EdgeTableBuffer);

  /*
   *      Scan down through the polygon edges, one scan line at a time,
   *      so long as at least one edge remains in either the GET or AET 
   */
  AETPtr = NULL; /* initialize the active edge table to empty */
  if (GETPtr != NULL
    )
    CurrentY = GETPtr->StartY; /* start at the top polygon vertex */
  while ((GETPtr != NULL) || (AETPtr != NULL)) {
    pf_move_xsorted_AET(CurrentY); /* update AET for this scan line */
    pf_scan_out_AET(CurrentY); /* draw this scan line from AET */
    pf_advance_AET(); /* advance AET edges 1 scan line */
    pf_xsort_AET(); /* resort on X */
    CurrentY++; /* advance to the next scan line */
  }

  tmp_free(EdgeTableBuffer);
}

/* 
 *   Creates a GET in the buffer pointed to by NextFreeEdgeStruc from
 *   the vertex list. Edge endpoints are flipped, if necessary, to
 *   guarantee all edges go top to bottom. The GET is sorted primarily
 *   by ascending Y start coordinate, and secondarily by ascending X
 *   start coordinate within edges with common Y coordinates 
 */
void pf_build_GET(ipt_t * VertexPtr, int ptNum, struct EdgeState *NextFreeEdgeStruc) {
  int i, StartX, StartY, EndX, EndY, DeltaY, DeltaX, Width, temp;
  struct EdgeState *NewEdgePtr;
  struct EdgeState *FollowingEdge;
  struct EdgeState **FollowingEdgeLink;

  /*
   *      Scan through the vertex list and put all non-0-height edges into
   *      the GET, sorted by increasing Y start coordinate 
   */
  GETPtr = NULL; /* initialize the global edge table to empty */
  for (i = 0; i < ptNum; i++) {
    /*
     * Calculate the edge height and width 
     */
    StartX = VertexPtr[i].x;
    StartY = VertexPtr[i].y;
    /*
     * The edge runs from the current point to the previous one 
     */
    if (i == 0) {
      /*
       * Wrap back around to the end of the list 
       */
      EndX = VertexPtr[ptNum - 1].x;
      EndY = VertexPtr[ptNum - 1].y;
    } else {
      EndX = VertexPtr[i - 1].x;
      EndY = VertexPtr[i - 1].y;
    }
    /*
     * Make sure the edge runs top to bottom 
     */
    if (StartY > EndY) {
      SWAP(StartX, EndX, temp);
      SWAP(StartY, EndY, temp);
    }
    /*
     * Skip if this can't ever be an active edge (has 0 height) 
     */
    if ((DeltaY = EndY - StartY) != 0) {
      /*
       * Allocate space for this edge's info, and fill in the structure 
       */
      NewEdgePtr = NextFreeEdgeStruc++;
      NewEdgePtr->XDirection = /* direction in which X moves */
      ((DeltaX = EndX - StartX) > 0) ? 1 : -1;
      Width = abs(DeltaX);
      NewEdgePtr->X = StartX;
      NewEdgePtr->StartY = StartY;
      NewEdgePtr->Count = DeltaY;
      NewEdgePtr->ErrorTermAdjDown = DeltaY;
      if (DeltaX >= 0) /* initial error term going L->R */
        NewEdgePtr->ErrorTerm = 0;
      else
        /* initial error term going R->L */
        NewEdgePtr->ErrorTerm = -DeltaY + 1;
      if (DeltaY >= Width) { /* Y-major edge */
        NewEdgePtr->WholePixelXMove = 0;
        NewEdgePtr->ErrorTermAdjUp = Width;
      } else { /* X-major edge */
        NewEdgePtr->WholePixelXMove = (Width / DeltaY) * NewEdgePtr->XDirection;
        NewEdgePtr->ErrorTermAdjUp = Width % DeltaY;
      }
      /*
       * Link the new edge into the GET so that the edge list is still sorted
       * by Y coordinate, and by X coordinate for all edges with the same Y
       * coordinate 
       */
      FollowingEdgeLink = &GETPtr;
      for (;;) {
        FollowingEdge = *FollowingEdgeLink;
        if ((FollowingEdge == NULL) || (FollowingEdge->StartY > StartY)
            || ((FollowingEdge->StartY == StartY) && (FollowingEdge->X >= StartX))) {
          NewEdgePtr->NextEdge = FollowingEdge;
          *FollowingEdgeLink = NewEdgePtr;
          break;
        }
        FollowingEdgeLink = &FollowingEdge->NextEdge;
      }
    }
  }
}

/* 
 *	Sorts all edges currently in the active edge table into ascending
 *	order of current X coordinates 
 */
void pf_xsort_AET() {
  struct EdgeState *CurrentEdge;
  struct EdgeState **CurrentEdgePtr;
  struct EdgeState *TempEdge;
  int SwapOccurred;

  /*
   * Scan through the AET and swap any adjacent edges for which the * second
   * edge is at a lower current X coord than the first edge.  Repeat until no
   * further swapping is needed 
   */
  if (AETPtr != NULL) {
    do {
      SwapOccurred = 0;
      CurrentEdgePtr = &AETPtr;
      while ((CurrentEdge = *CurrentEdgePtr)->NextEdge != NULL) {
        if (CurrentEdge->X > CurrentEdge->NextEdge->X) {
          /*
           * The second edge has a lower X than the first; swap them in the AET 
           */
          TempEdge = CurrentEdge->NextEdge->NextEdge;
          *CurrentEdgePtr = CurrentEdge->NextEdge;
          CurrentEdge->NextEdge->NextEdge = CurrentEdge;
          CurrentEdge->NextEdge = TempEdge;
          SwapOccurred = 1;
        }
        CurrentEdgePtr = &(*CurrentEdgePtr)->NextEdge;
      }
    } while (SwapOccurred != 0);
  }
}

/* 
 *	Advances each edge in the AET by one scan line.
 *	Removes edges that have been fully scanned. 
 */
void pf_advance_AET() {
  struct EdgeState *CurrentEdge;
  struct EdgeState **CurrentEdgePtr;

  /*
   * Count down and remove or advance each edge in the AET 
   */
  CurrentEdgePtr = &AETPtr;
  while ((CurrentEdge = *CurrentEdgePtr) != NULL) {
    /*
     * Count off one scan line for this edge 
     */
    if ((--(CurrentEdge->Count)) == 0) {
      /*
       * This edge is finished, so remove it from the AET 
       */
      *CurrentEdgePtr = CurrentEdge->NextEdge;
    } else {
      /*
       * Advance the edge's X coordinate by minimum move 
       */
      CurrentEdge->X += CurrentEdge->WholePixelXMove;
      /*
       * Determine whether it's time for X to advance one extra 
       */
      if ((CurrentEdge->ErrorTerm += CurrentEdge->ErrorTermAdjUp) > 0) {
        CurrentEdge->X += CurrentEdge->XDirection;
        CurrentEdge->ErrorTerm -= CurrentEdge->ErrorTermAdjDown;
      }
      CurrentEdgePtr = &CurrentEdge->NextEdge;
    }
  }
}

/* 
 *	Moves all edges that start at the specified Y coordinate from the
 *	GET to the AET, maintaining the X sorting of the AET. 
 */
void pf_move_xsorted_AET(int YToMove) {
  struct EdgeState *AETEdge;
  struct EdgeState **AETEdgePtr;
  struct EdgeState *TempEdge;
  int CurrentX;

  /*
   * The GET is Y sorted. Any edges that start at the desired Y
   * coordinate will be first in the GET, so we'll move edges from
   * the GET to AET until the first edge left in the GET is no longer
   * at the desired Y coordinate. Also, the GET is X sorted within
   * each Y coordinate, so each successive edge we add to the AET is
   * guaranteed to belong later in the AET than the one just added 
   */
  AETEdgePtr = &AETPtr;
  while ((GETPtr != NULL) && (GETPtr->StartY == YToMove)) {
    CurrentX = GETPtr->X;
    /*
     * Link the new edge into the AET so that the AET is still sorted by X
     * coordinate 
     */
    while (1) {
      AETEdge = *AETEdgePtr;
      if ((AETEdge == NULL) || (AETEdge->X >= CurrentX)) {
        TempEdge = GETPtr->NextEdge;
        *AETEdgePtr = GETPtr; /* link the edge into the AET */
        GETPtr->NextEdge = AETEdge;
        AETEdgePtr = &GETPtr->NextEdge;
        GETPtr = TempEdge; /* unlink the edge from the GET */
        break;
      } else {
        AETEdgePtr = &AETEdge->NextEdge;
      }
    }
  }
}

/* 
 *	Fills the scan line described by the current AET at the specified Y
 *	coordinate in the specified color, using the odd/even fill rule.
 */
void pf_scan_out_AET(int YToScan) {
  int LeftX;
  struct EdgeState *CurrentEdge;

  /*
   * Scan through the AET, drawing line segments as each pair of edge
   * crossings is encountered. The nearest pixel on or to the right
   * of left edges is drawn, and the nearest pixel to the left of but
   * not on right edges is drawn 
   */
  CurrentEdge = AETPtr;
  while (CurrentEdge != NULL) {
    LeftX = CurrentEdge->X;
    CurrentEdge = CurrentEdge->NextEdge;
    /*
     * 8/5/95, NDC: Zen's bug 
     */
    if (CurrentEdge->X)
      dev_line(LeftX, YToScan, CurrentEdge->X - 1, YToScan);
    CurrentEdge = CurrentEdge->NextEdge;
  }
}
