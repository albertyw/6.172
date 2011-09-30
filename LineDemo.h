#ifndef LINE_DEMO_H
#define LINE_DEMO_H

#include "Line.h"
#include "CollisionWorld.h"

using namespace std;

class LineDemo
{
protected:
   // Iteration counter
   unsigned int count;

   // Number of frames to compute
   unsigned int numFrames;

   // Objects for line simulation
   CollisionWorld *collisionWorld;

   // Add lines for line simulation at beginning
   void createLines();

   // Remove lines from line simulation at the end
   void deleteLines();

public:

   // Constructor
   LineDemo();

   // Destructor
   ~LineDemo();

   // Set number of frames to compute
   void setNumFrames(unsigned int _numFrames);

   // Initialize line simulation
   void initLine();

   // Get i-th line
   Line *getLine(unsigned int index);

   // Get num of lines
   unsigned int getNumOfLines();

   // Get number of line-wall collisions
   unsigned int getNumLineWallCollisions();

   // Get number of line-line collisions
   unsigned int getNumLineLineCollisions();

   // Line simulation update function
   bool update();
};

#endif /* LINE_DEMO_H */

