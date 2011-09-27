// nbodies simulation
// Copyright 2009 Cilk Arts
//
// Modified by Cy Chan for 6.172

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <cmath>

/* Headers for image and random number generation */

extern "C++" {
#include <png.hpp>
#include "MersenneTwister.h"
}

#include <cilktools/cilkview.h>
//tbb mutex support TODO
//#include <cilk_mutex.h>

#ifndef M_PI
#define M_PI 3.14159265359
#endif

/* Constants */

#define MAXW 500             // image width
#define MAXH 500             // image height
#define NBODIES 800          // default number of bodies in simulation
#define NIMAGES 100          // default number of images to generate
#define NSTEPS 40            // number of simulation steps between images
#define SCALE 12             // universe = MAXW * SCALE, MAXX * SCALE
#define TIME_QUANTUM 0.2     // integration time step
#define GRAVITY 0.2          // gravitational constant

#define SEED1                // seed random number generator with 1

/* Typedefs and Structs */

typedef png::rgba_pixel Pixel;
typedef png::image< Pixel > Image;
typedef unsigned char uchar;

struct Body {
    double x;       // x position
    double y;       // y position
    double xv;      // x velocity
    double yv;      // y velocity
    double xf;      // x force
    double yf;      // y force
    double mass;    // mass
    double density; // density
    Pixel color;    // color
};

/* Function Declarations */

/* Calculate force between bodies bi and bj and return result in fx and fy */
void calculate_force(double *fx, double *fy, const Body &bi, const Body &bj);

// Add force, (fx,fy) to body b
void add_force(Body* b, double fx, double fy);
