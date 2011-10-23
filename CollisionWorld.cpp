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
   timeStep = 0.5;
   maxQuadTreeRecursions = 8;
   minElementsToSplit = 10;
}


// Update the lines.
void CollisionWorld::updateLines()
{
   detectIntersection();
   updatePosition();
   lineWallCollision();
}

//****************************NEW FUNCTIONS BELOW HERE*********************


// **TODO** NOT SURE HOW TO REPRESENT LINES.  SHOULD IT BE "vector<Line*> currentLines"?

// Run the quadTree collision detection
void CollisionWorld::quadTree(float xMax, float xMin, float yMax, float yMin, vector<Line*> currentLines, int recursions)
{
  if(recursions >= maxQuadTreeRecursions || currentLines.size() < minElementsToSplit){
    detectIntersectionNewSame(currentLines);
    return;
  }
  vector<Line*> leafLines;
  // Create 4 arrays/vectors to hold lines for child quadtree boxes
  vector<Line*> quad1;
  vector<Line*> quad2;
  vector<Line*> quad3; // Can we combine these vector statements for memory allocation optimization?
  vector<Line*> quad4;
  // for each line
  LineLocation location;
  //printf("%f ",xMin);
  //printf("%f ",xMax);
  //printf("%f ",yMin);
  //printf("%f ",yMax);
  //printf("%zu ", currentLines.size());
  for (vector<Line*>::iterator it = currentLines.begin(); it != currentLines.end(); ++it) {
    Line *line = *it;
    // check where line should exist (lineInsideQuadrant)
    location = lineInsideQuadrant(xMax, xMin, yMax, yMin, line);
    // if line is in a child quadtree add the line to one of the 4 arrays/vectors
    if(location == QUAD1){
      quad1.push_back(line); // These push_backs can probably be optimized, since push_back changes the vector size each time
    }else if(location == QUAD2){
      quad2.push_back(line);
    }else if(location == QUAD3){
      quad3.push_back(line);
    }else if(location == QUAD4){
      quad4.push_back(line);
    }else if(location == LEAF){
      leafLines.push_back(line);
    }else if(location == OUTSIDE){
      printf("p1X:%f\n",(*line).p1.x);
      printf("p1Y:%f\n",(*line).p1.y);
      printf("p2X:%f\n",(*line).p2.x);
      printf("p2Y:%f\n\n",(*line).p2.y);
      throw std::runtime_error::runtime_error("Bad Line passed down quadtree ");
    }
  }
  //printf("NEW ITERATION, num recurse: %i ", (recursions + 1));
  //printf("%zu %zu %zu %zu %zu\n", quad1.size(), quad2.size(), quad3.size(), quad4.size(), leafLines.size());
  
  // Spawn 4 recursions of quadTree with the 4 arrays/vectors of lines
  float xAvg = (xMax + xMin)/2;
  float yAvg = (yMax + yMin)/2;
  ++recursions;
  vector<Line*>::size_type vectorMin = 1;
  if(quad1.size() > vectorMin) quadTree(xMax, xAvg, yMax, yAvg, quad1, recursions);
  if(quad2.size() > vectorMin) quadTree(xAvg, xMin, yMax, yAvg, quad2, recursions);
  if(quad3.size() > vectorMin) quadTree(xAvg, xMin, yAvg, yMin, quad3, recursions);
  if(quad4.size() > vectorMin) quadTree(xMax, xAvg, yAvg, yMin, quad4, recursions);
  // Check for intersections within this box
  detectIntersectionNewSame(leafLines);
  // Check child boxes' lines with current box's lines
  detectIntersectionNew(leafLines, quad1);
  detectIntersectionNew(leafLines, quad2);
  detectIntersectionNew(leafLines, quad3);
  detectIntersectionNew(leafLines, quad4);
}

// Given a quadtree box and a line, find if a line is inside of a quadrant
// Use LineLocations
// Return OUTSIDE if line is outside of box
// Return LEAF if line is inside of box, but not quadrantable
// Return QUAD1 if line is inside first quadrant (between xMax, xAvg, yMax, yAvg)
// Return QUAD2 if line is inside second quadrant (between xAvg, xMin, yMax, yAvg)
// Return QUAD3 if line is inside third quadrant (between xAvg, xMin, yAvg, yMin)
// Return QUAD4 if line is insde fourth quadrant (between xMax, xAvg, yAvg, yMin)
// If a line is exactly on a quadrant border (i.e. one of the axes), return LEAF
LineLocation CollisionWorld::lineInsideQuadrant(float xMax, float xMin, float yMax, float yMin, Line *line)
{
   LineLocation location = OUTSIDE;
   // Math Stuff
   float xAvg = (xMax + xMin) / 2;
   float yAvg = (yMax + yMin) / 2;
   float xVel = fabs((*line).vel.x);
   float yVel = fabs((*line).vel.y);

   double xMinVec = std::min((*line).p1.x, (*line).p2.x);
   double xMaxVec = std::max((*line).p1.x, (*line).p2.x);
   double yMinVec = std::min((*line).p1.y, (*line).p2.y);
   double yMaxVec = std::max((*line).p1.y, (*line).p2.y);

   //test whether any point is outside the rectangle
   if(xMinVec + (xVel * 2) < xMin || xMaxVec - (xVel * 2) > xMax || yMinVec + (yVel * 2) < yMin || yMaxVec - (yVel * 2) > yMax)
      return LEAF;

   //test whether vector crosses quadrants
   bool left = (xMaxVec < xAvg && xMinVec < xAvg);
   bool right = (xMaxVec > xAvg && xMinVec > xAvg);
   bool up = (yMaxVec > yAvg && yMinVec > yAvg); 
   bool down = (yMaxVec < yAvg && yMinVec < yAvg);   
   
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

// Test all line-line pairs to see if they will intersect before the next time
// step.
void CollisionWorld::detectIntersection()
{
   // Use the quadTree function instead of the default slow implementation
   
   /*
   printf("%zu\n\n", lines.size());
   printf("p1X:%f\n",(*lines[0]).p1.x);
   printf("p1Y:%f\n",(*lines[0]).p1.y);
   printf("p2X:%f\n",(*lines[0]).p2.x);
   printf("p2Y:%f\n\n",(*lines[0]).p2.y);
   */
   quadTree(BOX_XMAX, BOX_XMIN, BOX_YMAX, BOX_YMIN, lines, 0);
   /*
   printf("p1X:%f\n",(*lines[0]).p1.x);
   printf("p1Y:%f\n",(*lines[0]).p1.y);
   printf("p2X:%f\n",(*lines[0]).p2.x);
   printf("p2Y:%f\n\n",(*lines[0]).p2.y);
   printf("FINISHED FRAME\n");
   */
   /*
   vector<Line*>::iterator it1, it2;
   for (it1 = lines.begin(); it1 != lines.end(); ++it1) {
      Line *l1 = *it1;
      for (it2 = it1+1; it2 != lines.end(); ++it2) {
         Line *l2 = *it2;
         IntersectionType intersectionType = intersect(l1, l2, timeStep);
         if (intersectionType != NO_INTERSECTION) {
            collisionSolver(l1, l2, intersectionType);
            numLineLineCollisions++;
         }
      }
   }
   */
}

// Test for intersection between each line in Line1 against each line in Lines2
void CollisionWorld::detectIntersectionNew(vector<Line*> Lines1, vector<Line*> Lines2)
{
  vector<Line*>::iterator it1, it2;
  for (it1 = Lines1.begin(); it1 != Lines1.end(); ++it1) {
    Line *l1 = *it1;
    for (it2 = Lines2.begin(); it2 != Lines2.end(); ++it2) {
      Line *l2 = *it2;
      if(l1 == l2) continue;
      //printf("%f\n", l2->vel.x);
      IntersectionType intersectionType = intersect(l1, l2, timeStep);
      //printf("%f\n", l2->vel.x);
      if (intersectionType != NO_INTERSECTION) {
        collisionSolver(l1, l2, intersectionType);
        numLineLineCollisions++;
        //printf("%f", l2->vel.x);
      }
      //printf("\n\n");
    }
  }
}
// Test for intersection between each line in Lines
void CollisionWorld::detectIntersectionNewSame(vector<Line*> Lines)
{
  vector<Line*>::iterator it1, it2;
  for (it1 = Lines.begin(); it1 != Lines.end(); ++it1) {
    Line *l1 = *it1;
    for (it2 = it1+1; it2 != Lines.end(); ++it2) {
       Line *l2 = *it2;
       IntersectionType intersectionType = intersect(l1, l2, timeStep);
       if (intersectionType != NO_INTERSECTION) {
         collisionSolver(l1, l2, intersectionType);
         numLineLineCollisions++;
       }
    }
  }
}


// ************* NEW FUNCTIONS ABOVE HERE ****************


// Update line positions.
void CollisionWorld::updatePosition()
{
   double t = timeStep;
   vector<Line*>::iterator it;
   for (it = lines.begin(); it != lines.end(); ++it) {
      Line *line = *it;
      line->p1 += (line->vel * t);
      line->p2 += (line->vel * t);
   }
}


void CollisionWorld::collisionSolver(Line *l1, Line *l2, IntersectionType
                                     intersectionType)
{
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
void CollisionWorld::lineWallCollision()
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


