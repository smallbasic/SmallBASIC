/*
*	ffill.c	5 May 95, updated for SB: June 2001
*
*	FloodFill
*
*	Source code is based on:
*	"Programmer's guide to PC & PS/2 Video Systems"
*/

#include "sys.h"
#include "device.h"

#if defined(OS_LIMITED)
#if defined(_PalmOS)
#define QUEUESIZE   160         /* The size of Queue (400=VGA) */
#else
#define QUEUESIZE   320         /* The size of Queue (400=VGA) */
#endif
#else
#define QUEUESIZE   2048
#endif

struct tagParams {
  word xl;                      /* leftmost pixel in run */
  word xr;                      /* rightmost pixel in run */
  int16 y;                      /* y-coordinate of run */
  byte f;                       /* TRUE if run is filled (blocked) */
};

struct tagQUEUE {
  struct tagQUEUE *pQ;          /* pointer to opposite queue */
  int d;                        /* direction (UP or DN) */
  int n;                        /* number of unfilled runs in queue */
  int h;                        /* index of head of queue */
  int t;                        /* index of tail of queue */
  struct tagParams *run;
};
typedef struct tagQUEUE QUEUE;

#define UP	    1
#define DN	   -1
#define FILLED	    1
#define NOT_FILLED  0
#define SCAN_UNTIL  0
#define SCAN_WHILE  1

word ff_scan_left(word, word, long, int) SEC(BIO);
word ff_scan_right(word, word, long, int) SEC(BIO);
word ff_next_branch(word, word, int) SEC(BIO);
void ff_add_queue(QUEUE *, word, word, int, int) SEC(BIO);
int ff_in_queue(QUEUE *, word, word, int) SEC(BIO);

static struct tagParams ff_buf1[QUEUESIZE];
static struct tagParams ff_buf2[QUEUESIZE];

/* Global variables of the module. */
static long ucBorder, ucFill;
static QUEUE Qup;
static QUEUE Qdn;
static int scan_type;

/*
*	fill
*/
void dev_ffill(word x0, word y0, long fill_color, long border_color)
{
  int y = y0, qp;
  int bChangeDirection;
  word x, xl = x0, xr = x0, xln, xrn;
  QUEUE *Q;
  long pcolor;

  pcolor = dev_fgcolor;
  dev_setcolor(fill_color);
  /*
   * do nothing if the seed pixel is a border pixel 
   */
  if (border_color == -1) {
    border_color = dev_getpixel(x0, y0);
    scan_type = SCAN_WHILE;
  }
  else
    scan_type = SCAN_UNTIL;

  if (scan_type == SCAN_UNTIL) {
    if (dev_getpixel(x0, y0) == border_color)
      return;
  }
  else {
    if (dev_getpixel(x0, y0) == fill_color)
      return;
  }
  Qup.run = ff_buf1;
  Qdn.run = ff_buf2;
  /*
   * save the border and fill values in global variables 
   */
  ucBorder = border_color;
  ucFill = fill_color;
  /*
   * initialize the queues 
   */
  Qup.pQ = &Qdn;                /* pointer to opposite queue */
  Qup.d = UP;                   /* direction for queue */
  Qup.h = -1;
  Qup.t = 0;

  Qdn.pQ = &Qup;
  Qdn.d = DN;
  Qdn.h = -1;
  Qdn.t = 0;

  /*
   * put the seed run in the up queue 
   */
  Q = &Qup;
  ff_add_queue(Q,
               ff_scan_left(x0, y0, ucBorder, scan_type),
               ff_scan_right(x0, y0, ucBorder, scan_type), y0, NOT_FILLED);

  /*
   * main loop 
   */
  for (;;) {
    /*
     * if the queue is empty, switch queues 
     */
    if (Q->n == 0) {
      Q = Q->pQ;
      /*
       * exit if the other queue is empty 
       */
      if (Q->n == 0)
        break;
    }
    /*
     * get the first non-filled run from the head of the queue 
     */
    qp = Q->h;
    while (qp >= Q->t) {
      if (!Q->run[qp].f) {
        y = Q->run[qp].y;
        xl = Q->run[qp].xl;
        xr = Q->run[qp].xr;
        /*
         * fill the run 
         */
        dev_line(xl, y, xr, y);
        Q->run[qp].f = FILLED;
        Q->n--;
        break;
      }
      else
        qp--;
    }
    /*
     * remove previously-filled runs from the current queue 
     */
    if (Q->d == UP)
      while ((Q->h > qp) && (Q->run[Q->h].y < (y - 1)))
        Q->h--;
    else
      while ((Q->h > qp) && (Q->run[Q->h].y > (y + 1)))
        Q->h--;

    /*
     * branch in the current direction 
     */
    xln = ff_next_branch(xl, xr, y + Q->d);
    while (xln != 0xFFFF) {
      /*
       * determine the starting point for ScanRight 
       */
      x = (xln > xl) ? xln : xl;

      /*
       * add the new branch if it's not already in the queue 
       */
      xrn = ff_scan_right(x, y + Q->d, ucBorder, scan_type);
      if (!ff_in_queue(Q, xln, xrn, y + Q->d))
        ff_add_queue(Q, xln, xrn, y + Q->d, NOT_FILLED);

      /*
       * if the new branch does not extend beyond the current run, 
       */
      /*
       * look for another branch 
       */
      if (xrn > (xr - 2))
        break;
      else {
        x = xrn + 2;
        xln = ff_next_branch(x, xr, y + Q->d);
      }
    }

    /*
     * branch in the opposite direction 
     */
    bChangeDirection = 0;
    xln = ff_next_branch(xl, xr, y - Q->d);
    while (xln != 0xFFFF) {
      /*
       * determine the starting point for ScanRight 
       */
      x = (xln > xl) ? xln : xl;

      /*
       * add the new branch to the opposite queue if it is not 
       */
      /*
       * already in the current queue 
       */
      xrn = ff_scan_right(x, y - Q->d, ucBorder, scan_type);
      if (!ff_in_queue(Q, xln, xrn, y - Q->d)) {
        ff_add_queue(Q->pQ, xln, xrn, y - Q->d, NOT_FILLED);
        bChangeDirection = 1;
      }
      /*
       * if the new branch does not extend beyond the current run, 
       */
      /*
       * look for another branch 
       */
      if (xrn > (xr - 2))
        break;
      else {
        x = xrn + 2;
        xln = ff_next_branch(x, xr, y - Q->d);
      }
    }
    /*
     * change direction if any turns were detected 
     */
    if (bChangeDirection) {
      /*
       * select the opposite queue 
       */
      Q = Q->pQ;

      /*
       * copy the current run to the opposite queue 
       */
      ff_add_queue(Q, xl, xr, y, FILLED);
    }
  }

  // cleanup
  dev_setcolor(pcolor);
}

/*
*/
word ff_scan_left(word xl, word y, long ucPixel, int f)
{
  long v;

  if (f == SCAN_UNTIL) {
    /*
     * return 0xFFFF if starting pixel is a border pixel 
     */
    if (dev_getpixel(xl, y) == ucPixel)
      return 0xFFFF;

    do {
      xl--;

      /*
       * clip 
       */
      if (xl == 0xFFFF || xl < dev_Vx1)
        break;

      v = dev_getpixel(xl, y);
    } while (v != ucPixel);
  }
  else {                        /* f == SCAN_WHILE */
    /*
     * return 0xFFFF if starting pixel is not a border pixel 
     */
    if (dev_getpixel(xl, y) != ucPixel)
      return 0xFFFF;

    do {
      xl--;

      /*
       * clip 
       */
      if (xl == 0xFFFF || xl < dev_Vx1)
        break;

      v = dev_getpixel(xl, y);
    } while (v == ucPixel);
  }

  return ++xl;
}

/*
*/
word ff_scan_right(word xr, word y, long ucPixel, int f)
{
  long v;

  if (f == SCAN_UNTIL) {
    /*
     * return 0xFFFF if starting pixel is a border pixel 
     */
    if (dev_getpixel(xr, y) == ucPixel)
      return 0xFFFF;

    do {
      xr++;

      /*
       * clip 
       */
      if (xr > dev_Vx2)
        break;

      v = dev_getpixel(xr, y);
    } while (v != ucPixel);
  }
  else {                        /* f == SCAN_WHILE */
    /*
     * return 0xFFFF if starting pixel is not a border pixel 
     */
    if (dev_getpixel(xr, y) != ucPixel)
      return 0xFFFF;

    do {
      xr++;

      /*
       * clip 
       */
      if (xr > dev_Vx2)
        break;

      v = dev_getpixel(xr, y);
    } while (v == ucPixel);
  }

  return --xr;
}

/*
*/
word ff_next_branch(word xl, word xr, int y)
{
  word xln;


  /*
   * clip y 
   */
  if ((y < dev_Vy1) || (y > dev_Vy2))
    return 0xFFFF;

  /*
   * look for a branch adjacent to the first pixel in the run 
   */
  xln = ff_scan_left(xl, y, ucBorder, scan_type);

  /*
   * if the adjacent pixel is on the border, scan for the 
   */
  /*
   * first non-border pixel 
   */
  if (xln == 0xFFFF) {
    xln =
      ff_scan_right(xl, y, ucBorder,
                    (scan_type == SCAN_WHILE) ? SCAN_UNTIL : SCAN_WHILE);
    if (xln < xr)
      xln++;
    else
      xln = 0xFFFF;
  }

  return xln;
}

/*
*/
void ff_add_queue(QUEUE * Q, word xl, word xr, int y, int f)
{
  int qp;
  int i;

  if (Q->d == UP) {
    /*
     * find the last larger queue y-value 
     */
    for (qp = Q->t; qp <= Q->h; qp++)
      if (Q->run[qp].y <= y)
        break;
  }
  else {
    /*
     * find the last smaller queue y-value 
     */
    for (qp = Q->t; qp <= Q->h; qp++)
      if (Q->run[qp].y >= y)
        break;
  }

  /*
   * expand the queue if necessary 
   */
  if (qp <= Q->h) {
    /*
     * update the head pointer 
     */
    Q->h++;

    /*
     * expand up 
     */
    for (i = Q->h; i > qp; --i) {
      Q->run[i].xl = Q->run[i - 1].xl;
      Q->run[i].xr = Q->run[i - 1].xr;
      Q->run[i].y = Q->run[i - 1].y;
      Q->run[i].f = Q->run[i - 1].f;
    }
  }
  else                          /* can't insert in queue; add to head */
    Q->h++;

  /*
   * enter the data in the queue 
   */
  Q->run[qp].xl = xl;
  Q->run[qp].xr = xr;
  Q->run[qp].y = y;
  Q->run[qp].f = f;

  /*
   * update the number of unfilled runs in the queue 
   */
  if (!f)
    Q->n++;
}

/*
*/
int ff_in_queue(QUEUE * Q, word xl, word xr, int y)
{
  int qp;
  int bRVal = 0;


  if (Q->d == UP) {
    /*
     * search from head to tail 
     */
    for (qp = Q->h; qp >= 0; --qp) {
      /*
       * quit searching if y is not in the ordered list 
       */
      if (Q->run[qp].y > y)
        break;

      /*
       * quit searching if the specified run is in the list 
       */
      if ((Q->run[qp].y == y) && (Q->run[qp].xl == xl) && (Q->run[qp].xr == xr)) {
        bRVal = 1;
        break;
      }
    }
  }

  else {
    /*
     * search from head to tail 
     */
    for (qp = Q->h; qp >= 0; --qp) {
      /*
       * quit searching if y is not in the ordered list 
       */
      if (Q->run[qp].y < y)
        break;

      /*
       * quit searching if the specified run is in the list 
       */
      if ((Q->run[qp].y == y) && (Q->run[qp].xl == xl) && (Q->run[qp].xr == xr)) {
        bRVal = 1;
        break;
      }
    }
  }

  return bRVal;
}
