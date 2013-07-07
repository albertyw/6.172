/*
 * CollisionWorld detects and handles the line segment intersections
 */

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdexcept>

#include "CollisionWorld.h"
#include "IntersectionDetection.h"
#include "Line.h"

// Constructor.
CollisionWorld::CollisionWorld()
{
   numLineWallCollisions = 0;
   numLineLineCollisions = 0;
   timeStep = 1;
   maxQuadTreeRecursions = 7;  // Maximum recursion depth
   minElementsToSplit = 25;    // Minimum quadtree size
}


// Update the lines.
void CollisionWorld::updateLines()
{
   detectIntersection();
   updatePosition();
   lineWallCollision();
}

// TODO: CHANGE TIMESTEP TO INT (use shifting)
// TODO: CHANGE DOUBLES AND FLOATS TO INTS
// TODO: TEST FOR PARALLEL MOTION IN LINES
// TODO: CHANGE COORDINATE SYSTEM TO INT
// TODO: PARALLELIZE PLACING LINES INTO BUCKETS
// TODO: TRY CACHING LOW-LEVEL FUNCTIONS LIKE CROSS PRODUCTS, AVOID DIVISION/MULTIPLICATION
// TODO: PARALLELIZE COLLISION
// TODO: CHANGE EVERYTHING TO LISTS
// TODO: INLINING


/****************************NEW FUNCTIONS BELOW HERE*********************/

/**
 * Run the quadTree collision detection
 * Return the number of LineLineCollisions Found
 **/
int CollisionWorld::quadTree(float xMax, float xMin, float yMax, float yMin, list<Line*> currentLines, int recursions)
{
  if(recursions >= maxQuadTreeRecursions ||      // Maximum recursion depth 
    currentLines.size() < minElementsToSplit){   // Minimum quadtree size
    detectIntersectionNewSame(currentLines);
    list<IntersectionInfo> intersections = detectIntersectionNewSame(currentLines);
    return allCollisionSolver(intersections);
  }
  float xAvg = (xMax + xMin)/2;
  float yAvg = (yMax + yMin)/2;
  
  // Create a vector to hold lines that must sit in the current node
  list<Line*> leafLines;
  // Create 4 vectors to hold lines for child quadtree boxes
  list<Line*> quad1, quad2, quad3, quad4;
  // for each line
  LineLocation location;
  for (list<Line*>::iterator it = currentLines.begin(); it != currentLines.end(); ++it) {
    // check where line should exist (lineInsideQuadrant)
    location = lineInsideQuadrant(xMax, xMin, xAvg, yMax, yMin, yAvg, *it);
    // if line is in a child quadtree add the line to one of the 4 arrays/vectors
    if(location == QUAD1){quad1.push_back(*it);
    }else if(location == QUAD2){quad2.push_back(*it);
    }else if(location == QUAD3){quad3.push_back(*it);
    }else if(location == QUAD4){quad4.push_back(*it);
    }else if(location == LEAF){leafLines.push_back(*it);
    }/*else{
      printf("X must be between %f and %f\n", xMin, xMax);
      //printf("p1X:%f\n",(*it).p1.x);
      //printf("p2X:%f\n",(*it).p2.x);
      printf("Y must be between %f and %f\n", yMin, yMax);
      //printf("p1Y:%f\n",(*it).p1.y);
      //printf("p2Y:%f\n\n",(*it).p2.y);
      throw std::runtime_error::runtime_error("Bad Line passed down quadtree ");
    }*/
  }
  
  // Spawn 4 recursions of quadTree with the 4 lists of lines
  ++recursions;
  int lineLineCollisions1 = 0;
  int lineLineCollisions2 = 0;
  int lineLineCollisions3 = 0;
  int lineLineCollisions4 = 0;
  list<Line*>::size_type vectorMin = 1;
  // Create child quadTrees
  if(quad1.size() > vectorMin) lineLineCollisions1 = cilk_spawn quadTree(xMax, xAvg, yMax, yAvg, quad1, recursions);
  if(quad2.size() > vectorMin) lineLineCollisions2 = cilk_spawn quadTree(xAvg, xMin, yMax, yAvg, quad2, recursions);
  if(quad3.size() > vectorMin) lineLineCollisions3 = cilk_spawn quadTree(xAvg, xMin, yAvg, yMin, quad3, recursions);
  if(quad4.size() > vectorMin) lineLineCollisions4 = cilk_spawn quadTree(xMax, xAvg, yAvg, yMin, quad4, recursions);
  cilk_sync;
  int lineLineCollisions = lineLineCollisions1 + lineLineCollisions2 + lineLineCollisions3 + lineLineCollisions4;
  
  // Check for intersections within this box
  list<IntersectionInfo> intersections = cilk_spawn detectIntersectionNewSame(leafLines);
  
  // Check child boxes' lines with current box's lines
  list<IntersectionInfo> intersectionsquad1 = cilk_spawn detectIntersectionNew(leafLines, quad1);
  list<IntersectionInfo> intersectionsquad2 = cilk_spawn detectIntersectionNew(leafLines, quad2);
  list<IntersectionInfo> intersectionsquad3 = cilk_spawn detectIntersectionNew(leafLines, quad3);
  list<IntersectionInfo> intersectionsquad4 = cilk_spawn detectIntersectionNew(leafLines, quad4);
  cilk_sync;
  
  lineLineCollisions += allCollisionSolver(intersections);
  lineLineCollisions += allCollisionSolver(intersectionsquad1);
  lineLineCollisions += allCollisionSolver(intersectionsquad2);
  lineLineCollisions += allCollisionSolver(intersectionsquad3);
  lineLineCollisions += allCollisionSolver(intersectionsquad4);
  return lineLineCollisions;
}


/**
 * Given a quadtree box and a line, find if a line is inside of a quadrant
 * Use LineLocations
 * Return OUTSIDE if line is outside of box
 * Return LEAF if line is inside of box, but not quadrantable
 * Return QUAD1 if line is inside first quadrant (between xMax, xAvg, yMax, yAvg)
 * Return QUAD2 if line is inside second quadrant (between xAvg, xMin, yMax, yAvg)
 * Return QUAD3 if line is inside third quadrant (between xAvg, xMin, yAvg, yMin)
 * Return QUAD4 if line is insde fourth quadrant (between xMax, xAvg, yAvg, yMin)
 * If a line is exactly on a quadrant border (i.e. one of the axes), return LEAF
 **/
inline LineLocation CollisionWorld::lineInsideQuadrant(float xMax, float xMin, float xAvg, float yMax, float yMin, float yAvg, Line *line){
   // Math Stuff
   double xMinVec = std::min(line->p1.x, line->p2.x);
   double xMaxVec = std::max(line->p1.x, line->p2.x);
   double yMinVec = std::min(line->p1.y, line->p2.y);
   double yMaxVec = std::max(line->p1.y, line->p2.y);

   //test whether any point is outside the rectangle
   // We don't need to test this anymore because quadTrees never gives bad lines to children
   //double buffer = 0.01; // Use this buffer because comparing floating points is not exact
   //if(xMinVec < xMin-buffer || xMaxVec > xMax+buffer || yMinVec < yMin-buffer || yMaxVec > yMax+buffer)
   //   return OUTSIDE;

   //test whether vector crosses quadrants
   bool left = (xMaxVec < xAvg && xMinVec < xAvg);
   bool right = (xMaxVec > xAvg && xMinVec > xAvg);
   bool up = (yMaxVec > yAvg && yMinVec > yAvg); 
   bool down = (yMaxVec < yAvg && yMinVec < yAvg);   
   
   // Return one of five values
   if(right && up)
      return QUAD1;
   else if(left && up)
      return QUAD2;
   else if(left && down)
      return QUAD3;
   else if(right && down)
      return QUAD4;
   else
      return LEAF;
}


/**
 * Test all line-line pairs to see if they will intersect before the next time
 * step.
 **/
void CollisionWorld::detectIntersection()
{
   // Use the quadTree function instead of the default slow implementation
   list<Line*> allLines;
   for (vector<Line*>::iterator it = lines.begin(); it != lines.end(); ++it) {
     allLines.push_back(*it);
   }
   numLineLineCollisions += quadTree(BOX_XMAX, BOX_XMIN, BOX_YMAX, BOX_YMIN, allLines, 0);
}


/**
 * Test for intersection between each line in Line1 against each line in Lines2
 **/
inline list<IntersectionInfo> CollisionWorld::detectIntersectionNew(list<Line*> Lines1, list<Line*> Lines2)
{
  list<IntersectionInfo> intersections;
  for (list<Line*>::iterator it1 = Lines1.begin(); it1 != Lines1.end(); ++it1) {
    list<Line*>::iterator it2;
    for (it2 = Lines2.begin(); it2 != Lines2.end(); ++it2) {
      IntersectionType intersectionType = intersect(*it1, *it2, timeStep);
      if (intersectionType != NO_INTERSECTION) {
         IntersectionInfo intersection = IntersectionInfo(*it1, *it2, intersectionType);
         intersections.push_back(intersection);
      }
    }
  }
  return intersections;
}

/**
 * Test for intersection between each line in Lines
 **/
inline list<IntersectionInfo> CollisionWorld::detectIntersectionNewSame(list<Line*> Lines){
  list<IntersectionInfo> intersections;
  for (list<Line*>::iterator it1 = Lines.begin(); it1 != Lines.end(); ++it1) {
    list<Line*>::iterator it2 = it1;
    ++it2;
    for (it2; it2 != Lines.end(); ++it2) {
       IntersectionType intersectionType = intersect(*it1, *it2, timeStep);
       if (intersectionType != NO_INTERSECTION) {
         IntersectionInfo intersection = IntersectionInfo(*it1, *it2, intersectionType);
         intersections.push_back(intersection);
       }
    }
  }
  return intersections;
}

/**
 * Solve all of the collisions in the list<IntersectionInfo>
 * Return the number of line line collisions found
 **/
inline int CollisionWorld::allCollisionSolver(list<IntersectionInfo> intersections){
  list<IntersectionInfo>::iterator i;
  for(i=intersections.begin(); i!=intersections.end(); ++i){// If we coarsen this loop, we can make it parallel
    collisionSolver(i->l1, i->l2, i->intersectionType);
  }
  return (int) intersections.size();
}


/************* NEW FUNCTIONS ABOVE HERE ****************/


// Update line positions.
inline void CollisionWorld::updatePosition()
{
   double t = timeStep;
   vector<Line*>::iterator it;
   for (it = lines.begin(); it != lines.end(); ++it) {
      Line *line = *it;
      line->p1 += (line->vel);
      line->p2 += (line->vel);
   }
}


inline void CollisionWorld::collisionSolver(Line *l1, Line *l2, IntersectionType
                                     intersectionType){
   // Despite our efforts to determine whether lines will intersect ahead of
   // time (and to modify their velocities appropriately), our simplified model
   // can sometimes cause lines to intersect.  In such a case, we compute
   // velocities so that the two lines can get unstuck in the fastest possible
   // way, while still conserving momentum and kinetic energy.
   if (intersectionType == ALREADY_INTERSECTED) {
      Vec p = getIntersectionPoint(l1->p1, l1->p2, l2->p1, l2->p2);

      if ((l1->p1 - p).length() < (l1->p2 - p).length()) {
         l1->vel = (l1->p2 - p).normalize() * l1->vel.length();
      } else {
         l1->vel = (l1->p1 - p).normalize() * l1->vel.length();
      }
      if ((l2->p1 - p).length() < (l2->p2 - p).length()) {
         l2->vel = (l2->p2 - p).normalize() * l2->vel.length();
      } else {
         l2->vel = (l2->p1 - p).normalize() * l2->vel.length();
      }
      return;
   }

   // Compute the collision face/normal vectors
   Vec face;
   Vec normal;
   if (intersectionType == L1_WITH_L2) {
      Vec v(*l2);
      face = v.normalize();
   } else {
      Vec v(*l1);
      face = v.normalize();
   }
   normal = face.orthogonal();

   // Obtain each line's velocity components with respect to the collision
   // face/normal vectors.
   double v1Face = l1->vel.dotProduct(face);
   double v2Face = l2->vel.dotProduct(face);
   double v1Normal = l1->vel.dotProduct(normal);
   double v2Normal = l2->vel.dotProduct(normal);

   // Compute the mass of each line (we simply use its length).
   double m1 = (l1->p1 - l1->p2).length();
   double m2 = (l2->p1 - l2->p2).length();

   // Perform the collision calculation (computes the new velocities along the
   // direction normal to the collision face such that momentum and kinetic
   // energy are conserved).
   double newV1Normal = ((m1 - m2) / (m1 + m2)) * v1Normal +
                        (2 * m2 / (m1 + m2)) * v2Normal;
   double newV2Normal = (2 * m1 / (m1 + m2)) * v1Normal +
                        ((m2 - m1) / (m2 + m1)) * v2Normal;

   // Combine the resulting velocities.
   l1->vel = normal * newV1Normal + face * v1Face;
   l2->vel = normal * newV2Normal + face * v2Face;

   return;
}


// Handle line to wall collisions
inline void CollisionWorld::lineWallCollision()
{
   vector<Line*>::iterator it;
   for (it = lines.begin(); it != lines.end(); ++it) {
      Line *line   = *it;
      bool collide = false;

      // Right side
      if ((line->p1.x > BOX_XMAX  ||
            line->p2.x > BOX_XMAX) &&
            (line->vel.x > 0)) {
         line->vel.x = -line->vel.x;
         collide  = true;
      }
      // Left side
      if ((line->p1.x < BOX_XMIN  ||
            line->p2.x < BOX_XMIN) &&
            (line->vel.x < 0)) {
         line->vel.x = -line->vel.x;
         collide  = true;
      }
      // Top side
      if ((line->p1.y >  BOX_YMAX  ||
            line->p2.y >  BOX_YMAX) &&
            (line->vel.y > 0)) {
         line->vel.y = -line->vel.y;
         collide  = true;
      }
      // Bottom side
      if ((line->p1.y < BOX_YMIN  ||
            line->p2.y < BOX_YMIN) &&
            (line->vel.y < 0)) {
         line->vel.y = -line->vel.y;
         collide  = true;
      }
      // Update total number of collisions
      if (collide == true) {
         numLineWallCollisions++;
      }
   }
}


// Return the total number of lines in the box
unsigned int CollisionWorld::getNumOfLines()
{
   return lines.size();
}


// Add a line into the box
void CollisionWorld::addLine(Line *line)
{
   lines.push_back(line);
}


// Get the i-th line from the box
Line *CollisionWorld::getLine(unsigned int index)
{
   if (index >= lines.size())
      return NULL;
   return lines[index];
}


// Delete all lines in the box
void CollisionWorld::deleteLines()
{
   for(vector<Line*>::iterator it = lines.begin(); it < lines.end(); it++){
      delete *it;
   }
   lines.clear();
}


// Get total number of line wall collisions
unsigned int CollisionWorld::getNumLineWallCollisions()
{
   return numLineWallCollisions;
}


// Get total number of Line Line collisions;
unsigned int CollisionWorld::getNumLineLineCollisions()
{
   return numLineLineCollisions;
}


