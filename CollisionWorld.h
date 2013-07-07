#ifndef COLLISION_WORLD_H
#define COLLISION_WORLD_H

#include <vector>
using namespace std;
#include <cilk/cilk.h>
#include <cilktools/cilkview.h>
#include <cilktools/cilkscreen.h>

#include "Line.h"
#include "IntersectionDetection.h"
#include <cilk/reducer_list.h>

/***************NEW FUNCTIONS BELOW HERE**********************/
typedef enum { OUTSIDE, LEAF, QUAD1, QUAD2, QUAD3, QUAD4 } LineLocation;
struct IntersectionInfo {
  Line *l1;
  Line *l2;
  IntersectionType intersectionType;
  IntersectionInfo(Line *l1, Line *l2, IntersectionType intersectionType) {
    this->l1 = l1;
    this->l2 = l2;
    this->intersectionType = intersectionType;
  }
};
/**************NEW FUNCTIONS ABOVE HERE*********************/



class CollisionWorld
{
  
protected:
   // The size of the collision world
   int boxWidth;
   int boxHeight;

   // Time step used for simulation
   int  timeStep;

   // Container holding all the lines
   vector<Line*> lines;

   // Record the total number of line wall collision
   unsigned int numLineWallCollisions;

   // Record the total number of line line intersection
   unsigned int numLineLineCollisions;
   
   // Maximum number of recursions of a quadtree before the it gives up and 
   // manually checks for collisions
   unsigned int maxQuadTreeRecursions;
   
   // Minimum number of elements in a quad tree before it gives up and 
   // manully checks for collisions
   vector<Line*>::size_type minElementsToSplit;
   

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



   /**** NEW FUNCTIONS BELOW HERE ****/

   // Run quadTree recursive function
   int quadTree(float xMax, float xMin, float yMax, float yMin, list<Line*> currentLines, int recursions);

   // Given a quadtree box and a line, find if a line is inside of a quadrant
   // Use LineLocations
   // Return OUTSIDE if line is outside of box
   // Return LEAF if line is inside of box, but not quadrantable
   // Return QUAD1 if line is inside first quadrant (between xMax, xAvg, yMax, yAvg)
   // Return QUAD2 if line is inside second quadrant (between xAvg, xMin, yMax, yAvg)
   // Return QUAD3 if line is inside third quadrant (between xAvg, xMin, yAvg, yMin)
   // Return QUAD4 if line is insde fourth quadrant (between xMax, xAvg, yAvg, yMin)
   // If a line is exactly on a quadrant border (i.e. one of the axes), return LEAF
   LineLocation lineInsideQuadrant(float xMax, float xMin, float xAvg, float yMax, float yMin, float yAvg, Line *line);
   
   // Test for intersection between each line in Line1 against each line in Lines2
   list<IntersectionInfo> detectIntersectionNew(list<Line*> Lines1, list<Line*> Lines2);
   
   // Test for intersection between each line in Lines
   list<IntersectionInfo> detectIntersectionNewSame(list<Line*> Lines);  
   
   /**
   * Solve all of the collisions in the list<IntersectionInfo>
   **/
  int allCollisionSolver(list<IntersectionInfo> intersections);
  
  void smallCollisionSolver(int *i, _List_iterator<IntersectionInfo> distance);
};



#endif  /* COLLISION_WORLD_H */
