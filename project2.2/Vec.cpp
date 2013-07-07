#include "Vec.h"
#include "Line.h"

Vec::Vec(Line l)
{
   *this = l.p1 - l.p2;
}
