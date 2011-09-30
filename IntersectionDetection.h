#ifndef INTERSECTION_DETECTION_H
#define INTERSECTION_DETECTION_H

#include "Line.h"
#include "Vec.h"

typedef enum {NO_INTERSECTION, L1_WITH_L2, L2_WITH_L1,
              ALREADY_INTERSECTED
             } IntersectionType;

// Detect if line l1 and l2 will be intersected in the next time step.
IntersectionType intersect(Line *l1, Line *l2, double time);

// Check if a point is in the parallelogram.
bool pointInParallelogram(Vec point, Vec p1, Vec p2,
                          Vec p3, Vec p4);

// Check if two lines are intersected.
bool intersectLines(Vec p1, Vec p2, Vec p3, Vec p4);

// Check the direction of two lines (pi, pj) and (pi, pk)
double direction(Vec pi, Vec pj, Vec pk);

// Check if a point pk is in the line segment (pi, pj)
bool onSegment(Vec pi, Vec pj, Vec pk);

// Calculate the cross product.
double crossProduct(double x1, double y1, double x2, double y2);

// Obtain the intersection point for two intersecting line segments.
Vec getIntersectionPoint(Vec p1, Vec p2, Vec p3, Vec p4);

#endif // INTERSECTION_DETECTION_H
