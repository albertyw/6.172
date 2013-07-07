#ifndef VEC_H
#define VEC_H

// A simple 2d vector library

#include <stdio.h>
#include <math.h>

struct Line;

struct Vec {
   double x;
   double y;

   Vec() { }

   Vec(double x, double y) {
      this->x = x;
      this->y = y;
   }

   Vec(Line l);

   double dotProduct(Vec v) {
      return x * v.x + y * v.y;
   }

   double angle(Vec v1) {
      Vec v2(x, y);
      v1 = v1.normalize();
      v2 = v2.normalize();

      return atan2(v2.y, v2.x) - atan2(v1.y, v1.x);
   }

   Vec orthogonal() {
      Vec v(-y, x);
      return v;
   }

   Vec pojectOnto(Vec v1) {
      Vec v2 = *this;
      return v2 * v2.dotProduct(v1);
   }

   double length() {
      Vec v(x, y);
      return sqrt(dotProduct(v));
   }

   Vec normalize() {
      double n = length();
      Vec v = *this;
      v /= n;
      return v;
   }

   void operator+=(const Vec v1) {
      x += v1.x;
      y += v1.y;
   }

   void operator-=(const Vec v1) {
      x -= v1.x;
      y -= v1.y;
   }

   Vec operator+(const Vec v1) {
      Vec v2 = *this;
      v2 += v1;
      return v2;
   }

   Vec operator-(const Vec v1) {
      Vec v2 = *this;
      v2 -= v1;
      return v2;
   }

   Vec operator-() {
      Vec v = *this;
      v.x = -v.x;
      v.y = -v.y;
      return v;
   }

   void operator*=(const double s) {
      x *= s;
      y *= s;
   }
   void operator/=(const double s) {
      x /= s;
      y /= s;
   }

   Vec operator*(const double s) {
      Vec v2 = *this;
      v2 *= s;
      return v2;
   }

   void print() {
      printf("%f; %f\n", x, y);
   }
};

#endif
