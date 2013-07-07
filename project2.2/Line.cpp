#include <stdlib.h>
#include <iostream>
#include "Line.h"
#include "LineDemo.h"

/*
 * The PROFILE_BUILD preprocessor define is used to indicate we are
 * building for profiling, so don't include any graphics or Cilk functions.
 */

#ifndef PROFILE_BUILD

#include "GraphicStuff.h"
#include <cilktools/cilkview.h>

#else

// use pragma to disable cilk TODO
#endif

using namespace std;

// For non-graphic version
void lineMain(LineDemo *lineDemo)
{
   // Loop for updating line movement simulation
   while (true) {
      if (!lineDemo->update()) {
         break;
      }
   }
}


int main(int argc, char** argv)
{
   int optchar;
   bool graphicDemoFlag = false, imageOnlyFlag = false;
   unsigned int numFrames = 1;

   // process command line options
   while ((optchar = getopt(argc, argv, "gi")) != -1) {
      switch (optchar) {
         case 'g':
            graphicDemoFlag = true;
            break;
         case 'i':
            imageOnlyFlag = true;
            graphicDemoFlag = true;
            break;
         default:
            cout << "Ignoring unrecognized option: " << optchar << endl;
            continue;
      }
   }

   if (!imageOnlyFlag) {
      // shift remaining arguments over
      int remaining_args = argc - optind;
      for(int i = 1; i <= remaining_args; ++i ) {
         argv[i] = argv[i+optind-1];
      }

      // check to make sure number of arguments is correct
      if (remaining_args != 1) {
         cout << "Usage: " << argv[0] << " [-g] [-i] <numFrames>" << endl;
         cout << "  -g : show graphics" << endl;
         cout << "  -i : show first image only (ignore numFrames)" << endl;
         exit(-1);
      }

      numFrames = atoi(argv[1]);
      cout << "Number of frames = " << numFrames << endl;
   }

   // Create and initialize the Line simulation environment
   LineDemo *lineDemo = new LineDemo();
   lineDemo->initLine();
   lineDemo->setNumFrames(numFrames);

#ifndef PROFILE_BUILD
   // start cilkview performance analysis
   cilkview_data_t start, end;
   __cilkview_query(start);

   // run demo
   if (graphicDemoFlag) {
      graphicMain(argc, argv, lineDemo, imageOnlyFlag);
   } else {
      lineMain(lineDemo);
   }

   // stop cilkview performance analysis
   __cilkview_query(end);
#else
   lineMain(lineDemo);
#endif

   // output results
   cout << lineDemo->getNumLineWallCollisions()
        << " Line-Wall Collisions" << endl;
   cout << lineDemo->getNumLineLineCollisions()
        << " Line-Line Collisions" << endl;

#ifndef PROFILE_BUILD
   cout << (end.time - start.time) / 1000.0
        << " seconds in total" << endl;

   // dump cilkview analysis information
   __cilkview_do_report(&start, &end, "linedemo", 
       CV_REPORT_WRITE_TO_LOG | CV_REPORT_WRITE_TO_RESULTS);
#endif

   // delete objects
   delete lineDemo;

   return 0;
}
