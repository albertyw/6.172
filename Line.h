#ifndef LINE_H
#define LINE_H

#include "Vec.h"

/* Lines' coordinates are stored in a box with these bounds
 *
 * We choose box coordinates in [.5, 1) to simulate fixed
 * point floating point accuracy to mitigate issues with
 * associativity of coordinate updates
 */
#define BOX_XMIN .5
#define BOX_XMAX 1
#define BOX_YMIN .5
#define BOX_YMAX 1

// graphics are displayed in a box of this size
#define WINDOW_WIDTH 1180
#define WINDOW_HEIGHT 800

struct Line {
   Vec p1;
   Vec p2;
   Vec vel;
   bool isGray;
};

// convert graphical window coordinates to box coordinates
inline void windowToBox(double *xout, double *yout, double x, double y)
{
   *xout = x / WINDOW_WIDTH  * ((double) BOX_XMAX - BOX_XMIN) + BOX_XMIN;
   *yout = y / WINDOW_HEIGHT * ((double) BOX_YMAX - BOX_YMIN) + BOX_YMIN;

   return;
}

// convert box coordinates to graphical window coordinates
inline void boxToWindow(double *xout, double *yout, double x, double y)
{
   *xout = (x - BOX_XMIN) / ((double) BOX_XMAX - BOX_XMIN) * WINDOW_WIDTH;
   *yout = (y - BOX_YMIN) / ((double) BOX_YMAX - BOX_YMIN) * WINDOW_HEIGHT;

   return;
}

// convert graphical window velocity to box velocity
inline void velocityWindowToBox(double *xout, double *yout, double x, double y)
{
   *xout = x / WINDOW_WIDTH  * ((double) BOX_XMAX - BOX_XMIN);
   *yout = y / WINDOW_HEIGHT * ((double) BOX_YMAX - BOX_YMIN);

   return;
}

#endif /* LINE_H */
