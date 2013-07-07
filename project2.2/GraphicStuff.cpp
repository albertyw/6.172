#include <stdlib.h>
#include <stdio.h>

#include "Line.h"
#include "LineDemo.h"
#include "GraphicStuff.h"

static LineDemo *gLineDemo = NULL;
XSegment *segments = NULL;

Display *display;
Window window, root, parent;
int screen, depth, visibility;
int windowwidth, windowheight;

static void drawLineSegments(Display *display, Drawable drawable)
{
   Line *line;
   unsigned int i, nsegments;
   double px1, py1, px2, py2;

   Colormap cmap;
   XGCValues gcval;
   XColor color, ignore;
   long fgcolor;
   GC gray, red;

   cmap = DefaultColormap(display, screen);
   XAllocNamedColor(display, cmap, "gray", &color, &ignore);
   fgcolor = color.pixel;
   gcval.foreground = fgcolor;
   gray = XCreateGC(display, window, GCForeground, &gcval);

   XAllocNamedColor(display, cmap, "dark red", &color, &ignore);
   fgcolor = color.pixel;
   gcval.foreground = fgcolor;
   red = XCreateGC(display, window, GCForeground, &gcval);


   nsegments = gLineDemo->getNumOfLines();
   if (segments == NULL) {
      segments  = new XSegment[nsegments];
   }
   XClearWindow(display, window);
   for (i = 0; i < nsegments; i++) {
      line = gLineDemo->getLine(i);

      // convert box coordinates to window coordinates
      boxToWindow(&px1, &py1, line->p1.x, line->p1.y);
      boxToWindow(&px2, &py2, line->p2.x, line->p2.y);

      // convert doubles to short ints and store into segments
      segments[0].x1 = (short) px1;
      segments[0].y1 = (short) py1;
      segments[0].x2 = (short) px2;
      segments[0].y2 = (short) py2;

      // set line color
      if (line->isGray)
         XDrawSegments(display, drawable, gray, segments, 1);
      else
         XDrawSegments(display, drawable, red, segments, 1);
   }
   XSync(display, 0);
}


static void checkEvent()
{
   XEvent event;
   bool block = false;

   while ((XPending(display) > 0) || (block == true)) {
      XNextEvent(display, &event);
      switch (event.type) {
      case ReparentNotify:
         if (event.xreparent.window != window ) break;
         XSelectInput(display, event.xreparent.parent,
                      StructureNotifyMask);
         XSelectInput(display, parent, 0);
         parent = event.xreparent.parent;
         break;

      case UnmapNotify:
         if ((event.xunmap.window != window) &&
               (event.xunmap.window != parent)) break;
         block = true;
         break;

      case VisibilityNotify:
         if (event.xvisibility.window != window) break;
         if (event.xvisibility.state == VisibilityFullyObscured) {
            block = true;
            break;
         }
         if ((event.xvisibility.state == VisibilityUnobscured) &&
               (visibility == 1)) {
            visibility = 0;
            block = false;
            break;
         }
         if (event.xvisibility.state == VisibilityPartiallyObscured) {
            visibility = 1;
            block = false;
         }
         break;

      case Expose:
         block = false;
         break;

      case MapNotify:
         if ((event.xmap.window != window) &&
               (event.xmap.window != parent)) break;
         block = false;
         break;

      case ConfigureNotify:
         if (event.xconfigure.window != window) break;
         if ((windowwidth == event.xconfigure.width) &&
               (windowheight == event.xconfigure.height))
            break;
         windowwidth = event.xconfigure.width;
         windowheight = event.xconfigure.height;
         XClearWindow(display, window);
         block = false;
         break;

      default:
         break;
      }
   }
}


static void graphicMainLoop(bool imageOnlyFlag)
{
   while (true) {
      checkEvent();
      drawLineSegments(display, window);
      if (!imageOnlyFlag && !gLineDemo->update()) {
         return;
      }
   }
}


static void graphicInit(int *argc, char **argv)
{
   // Initialization
   long fgcolor, bgcolor;
   long eventmask;
   char *host;

   if ((host = (char *) getenv("DISPLAY")) == NULL) {
      perror("Error: No environment variable DISPLAY\n");
      exit(1);
   }

   if (!(display = XOpenDisplay(host))) {
      perror("XOpenDisplay");
      exit(1);
   }

   screen = DefaultScreen(display);
   root = RootWindow(display, screen);
   parent = root;
   bgcolor = BlackPixel(display, screen);
   fgcolor = WhitePixel(display, screen);
   depth = DefaultDepth(display, screen);
   windowwidth = WINDOW_WIDTH;
   windowheight = WINDOW_HEIGHT;
   window = XCreateSimpleWindow(display, root,
                                0, 0, windowwidth, windowheight,
                                2, fgcolor, bgcolor);

   eventmask = SubstructureNotifyMask;
   XSelectInput(display, window, eventmask);

   XMapWindow(display, window);

   XClearWindow(display, window);
   XSync(display, 0);
}


void graphicMain(int argc, char **argv, LineDemo *lineDemo, bool imageOnlyFlag)
{
   gLineDemo = lineDemo;

   // Initialization
   graphicInit(&argc, argv);

   // Entering the rendering loop
   graphicMainLoop(imageOnlyFlag);
}
