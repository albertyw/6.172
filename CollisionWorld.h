#ifndef COLLISION_WORLD_H
#define COLLISION_WORLD_H

#include <vector>
using namespace std;

#include "Line.h"
#include "IntersectionDetection.h"
#include <cilk/reducer_list.h>
class CollisionWorld
{
protected:
   // The size of the collision world
   int boxWidth;
   int boxHeight;

   // Time step used for simulation
   double  timeStep;

   // Container holding all the lines
   vector<Line*> lines;

   // Record the total number of line wall collision
   unsigned int numLineWallCollisions;

   // Record the total number of line line intersection
   unsigned int numLineLineCollisions;

public:
   CollisionWorld();

   // Return the total number of lines in the box
   unsigned int getNumOfLines();

   // Add a line into the box
   void  addLine(Line *line);

   // Get a line from box
   Line *getLine(unsigned int index);

   // Delete all lines in the box
   void  deleteLines();

   // Update lines' situation in the box
   void  updateLines();

   // Update position of lines
   void  updatePosition();

   // Handle line wall collision
   void lineWallCollision();

   // Detect Line Line intersection
   void  detectIntersection();

   // Get total number of line wall collisions
   unsigned int getNumLineWallCollisions();

   // Get total number of line line intersection
   unsigned int getNumLineLineCollisions();

   void collisionSolver(Line *l1, Line *l2, IntersectionType intersectionType);



   // **** NEW FUNCTIONS BELOW HERE ****

   // Run quadTree
   void quadTree();
};



#endif  /* COLLISION_WORLD_H */
