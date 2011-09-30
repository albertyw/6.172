/*
 * CollisionWorld detects and handles the line segment intersections
 */

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>

#include "CollisionWorld.h"
#include "IntersectionDetection.h"
#include "Line.h"

// Constructor.
CollisionWorld::CollisionWorld()
{
   numLineWallCollisions = 0;
   numLineLineCollisions = 0;
   timeStep = 0.5;
}


// Update the lines.
void CollisionWorld::updateLines()
{
   detectIntersection();
   updatePosition();
   lineWallCollision();
}


// Test all line-line pairs to see if they will intersect before the next time
// step.
void CollisionWorld::detectIntersection()
{
   vector<Line*>::iterator it1, it2;
   for (it1 = lines.begin(); it1 != lines.end(); ++it1) {
      Line *l1 = *it1;
      for (it2 = it1 + 1; it2 != lines.end(); ++it2) {
         Line *l2 = *it2;
         IntersectionType intersectionType = intersect(l1, l2, timeStep);
         if (intersectionType != NO_INTERSECTION) {
            collisionSolver(l1, l2, intersectionType);
            numLineLineCollisions++;
         }
      }
   }
}


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


