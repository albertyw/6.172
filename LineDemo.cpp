/*
 * LineDemo is the main driver of the line simulation
 */

#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "Line.h"
#include "LineDemo.h"

// The main simulation loop
bool LineDemo::update()
{
   count++;
   collisionWorld->updateLines();
   if (count > numFrames) {
      return false;
   }
   return true;
}


// Read in lines from line.in and add them into collision world for simulation
void LineDemo::createLines()
{
   double px1, py1, px2, py2, vx, vy;
   int isGray;
   FILE *fin;
   fin = fopen("line.in", "r");
   assert(fin != NULL);

   while (EOF != fscanf(fin, "(%lf, %lf), (%lf, %lf), %lf, %lf, %d\n",
                        &px1, &py1, &px2, &py2, &vx, &vy, &isGray)) {

      Line *line = new Line;

      // convert window coordinates to box coordinates
      windowToBox(&line->p1.x, &line->p1.y, px1, py1);
      windowToBox(&line->p2.x, &line->p2.y, px2, py2);

      // convert window velocity to box velocity
      velocityWindowToBox(&line->vel.x, &line->vel.y, vx, vy);

      // store color
      line->isGray = isGray;

      collisionWorld->addLine(line);
   }
   fclose(fin);
}


// Delete all the lines in collision world at end of simulation
void LineDemo::deleteLines()
{
   collisionWorld->deleteLines();
}


LineDemo::LineDemo()
{
   count = 0;
   collisionWorld = new CollisionWorld();
}


LineDemo::~LineDemo()
{
   delete collisionWorld;
}


void LineDemo::setNumFrames(unsigned int _numFrames)
{
  numFrames = _numFrames;
}

void LineDemo::initLine()
{
   createLines();
}

Line *LineDemo::getLine(unsigned int index)
{
   return collisionWorld->getLine(index);
}

unsigned int LineDemo::getNumOfLines()
{
   return collisionWorld->getNumOfLines();
}

unsigned int LineDemo::getNumLineWallCollisions()
{
   return collisionWorld->getNumLineWallCollisions();
}

unsigned int LineDemo::getNumLineLineCollisions()
{
   return collisionWorld->getNumLineLineCollisions();
}
