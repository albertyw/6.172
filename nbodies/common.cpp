// nbodies simulation
// Copyright 2009 Cilk Arts
//
// Modified by Cy Chan for 6.172

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <cmath>

#include "common.h"

/* Globals */

Pixel bkgnd(240,240,240);       // default background color
MTRand rnd;                     // random number generator object

/* Function Declarations */

// These functions are implemented in the files nbodies_X.cilk, where
// X is "loops", "symmetric", or "nolocks".

// Calculate forces between all of the bodies in the simulation for all pairs
void calculate_forces(int nbodies, Body *bodies);

// Given sums of forces acting on all of the bodies, update their positions
void update_positions(int nbodies, Body *bodies);

/* Function Definitions */

// Ensure that abs(r) >= THRESHOLD, preserving the sign. 
// This avoids the singularity of gravitational forces for r=0
static inline double make_nonzero(double r)
{
    const double THRESHOLD = 1.0e-100;
    if (r > 0) {
	if (r < THRESHOLD)
	    r = THRESHOLD;
    } else {
	if (r > -THRESHOLD)
	    r = -THRESHOLD;
    }
    return r;
}

// Calculate force between bodies bi and bj and return result in fx and fy
void calculate_force(double *fx, double *fy, const Body &bi, const Body &bj)
{
    double dx = make_nonzero(bj.x - bi.x);
    double dy = make_nonzero(bj.y - bi.y);

    // compute distance between bodies
    double dist2 = dx * dx + dy * dy;
    double dist = std::sqrt( dist2 );

    // law of gravitation
    double f = bi.mass * bj.mass * GRAVITY / dist2;

    // separate force into components
    (*fx) = f * dx / dist;
    (*fy) = f * dy / dist;
}

// Add force, (fx,fy) to body b
void add_force(Body* b, double fx, double fy)
{
    b->xf += fx;
    b->yf += fy;
}

// For debugging
void dumpbody(Body &b)
{
    std::cout << " [x,y " << b.x << " " << b.y << "] [xv,yv " << b.xv << " "
              << b.yv << "] [xf,yf " << b.xf << " " << b.yf << "] mass "
              << b.mass << std::endl;
}

// For debugging
void dumpbodies(int nbodies, Body *bodies)
{
    for (int i=0; i<nbodies; ++i)
    {
        std::cout << "[body " << i << "] ";
        dumpbody(bodies[i]);
    }
}

// draw a circle on the screen
// Algorithm from: http://www.cs.unc.edu/~mcmillan/comp136/Lecture7/circle.html

// fill in solid block of circle
// for each point on the perimeter, fill a line of pixels from center to edge
void block_points(int cx, int x, int y, int maxx, int maxy,
                 Image &img, Pixel color)
{
    if (x <= cx) {
        for (int i = x; i<= cx; ++i)
            if (i >= 0 && y >= 0 && i < maxx && y < maxy)
                img.set_pixel(i, y, color);
    }
    else {
        for (int i=x; i> cx; --i)
            if (i >= 0 && y >= 0 && i < maxx && y < maxy)
                img.set_pixel(i, y, color);
    }
}

// calculate all perimeter points
void circle_points(int cx, int cy, int x, int y, Image &img, Pixel color)
{
    int maxx = img.get_width();
    int maxy = img.get_height();

    if (cx + x < 0 || cy + y < 0 || cx + x > maxx || cy + y > maxy) return;
    if (x == 0) {
        block_points(cx, cx, cy + y, maxx, maxy, img, color);
        block_points(cx, cx, cy - y, maxx, maxy, img, color);
        block_points(cx, cx + y, cy, maxx, maxy, img, color);
        block_points(cx, cx - y, cy, maxx, maxy, img, color);
    } else if (x == y) {
        block_points(cx, cx + x, cy + y, maxx, maxy, img, color);
        block_points(cx, cx - x, cy + y, maxx, maxy, img, color);
        block_points(cx, cx + x, cy - y, maxx, maxy, img, color);
        block_points(cx, cx - x, cy - y, maxx, maxy, img, color);
    } else if (x < y) {
        block_points(cx, cx + x, cy + y, maxx, maxy, img, color);
        block_points(cx, cx - x, cy + y, maxx, maxy, img, color);
        block_points(cx, cx + x, cy - y, maxx, maxy, img, color);
        block_points(cx, cx - x, cy - y, maxx, maxy, img, color);
        block_points(cx, cx + y, cy + x, maxx, maxy, img, color);
        block_points(cx, cx - y, cy + x, maxx, maxy, img, color);
        block_points(cx, cx + y, cy - x, maxx, maxy, img, color);
        block_points(cx, cx - y, cy - x, maxx, maxy, img, color);
    }
}


void circle_midpoint(int xCenter, int yCenter, int radius,
                    Image &img, Pixel color)
{
    // scale to image space
    int xc = xCenter / SCALE;
    int yc = yCenter / SCALE;
    int y = radius / SCALE;

    int x = 0;
    int p = (5 - y*4)/4;

    circle_points(xc, yc, x, y, img, color);
    while (x < y) {
        x++;
        if (p < 0) {
            p += 2*x+1;
        } else {
            y--;
            p += 2*(x-y)+1;
        }
        circle_points(xc, yc, x, y, img, color);
    }
}

void update_picture(int maxw, int maxh, int nbodies,
                    Body *bodies, Pixel bkgnd_color, Image &img)
{

    // set background
    for (int i=0; i<maxw; ++i) {
        for (int j=0; j<maxh; ++j)
        {
            img.set_pixel(i, j, bkgnd_color);
        }
    }

    // draw each body into the image (radius depends on mass)
    for (int i = 0; i<nbodies; ++i)
    {
        int radius = int(std::sqrt((bodies[i].mass / bodies[i].density) / 
				   M_PI));
        circle_midpoint(int(bodies[i].x), int(bodies[i].y), radius,
                        img, bodies[i].color);
    }
}

void initialize_bodies(int nbodies, Body *bodies, int maxx, int maxy)
{
    const double mx = double(maxx); 
    const double my = double(maxy);
    const double sx = mx / 2.0;
    const double sy = my / 2.0;

    // Place the "sun" in the middle
    bodies[0].x = sx;
    bodies[0].y = sy;
    bodies[0].xv = 0.0;
    bodies[0].yv = 0.0;
    bodies[0].xf = 0.0;
    bodies[0].yf = 0.0;
    bodies[0].mass = 250000.0;
    bodies[0].density = 5;
    bodies[0].color = Pixel(0,0,0);

    // initialize the bodies - in 'pixel space' coordinates

    // Minimum distance any body should be from the sun
    const double min_d = mx / 8.0;

    // N-body random positions
    for (int i=1; i<nbodies; ++i)
    {
        Body &b = bodies[i];

        // find a point at least min_distance from "sun"
        double d, dx, dy;
        do {
            b.x = rnd.rand(mx);
            b.y = rnd.rand(my);
            dx = b.x - sx;
            dy = b.y - sy;
            d = std::sqrt(dx*dx + dy*dy);
        } while (d < min_d);

        const double V = 2;  // Max velocity

        // give everything random clockwise torque in range V/3 to V
        // NOTE: picture uses a left-handed coordinate system, so the sign of
        // velocities are reversed vs. a right-handed coordinate system.
        double v_abs = V * (1.0/3.0 + rnd.rand(2.0/3.0));
        b.xv = -v_abs * dy / d;
        b.yv = v_abs * dx / d;

        // Scale velocities by distance (closer moves faster)
        b.xv = b.xv * mx/d;
        b.yv = b.yv * mx/d;

        b.xf = 0.0;
        b.yf = 0.0;
        b.mass = 25.0 + rnd.rand(175.0);  // Mass from 25 to 200
        b.density = 0.02;
        b.color = Pixel(uchar(rnd.randInt(255)), uchar(rnd.randInt(255)),
                        uchar(rnd.randInt(200)));  // Random color (not white)
    }

}

int main(int argc, char* argv[])
try
{
    int nbodies = NBODIES;       // default
    if (argc > 1) {
        nbodies = atoi(argv[1]);
        if (nbodies <= 0 || nbodies > 20000) {
            std::cerr << "Usage: nbodies [nbodies] [nimages] "
                      << "where 0 < nbodies < 20001"
                      << std::endl;
            return 1;
        }
    }

    int nimages = NIMAGES;       // default
    if (argc > 2) {
        nimages = atoi(argv[2]);
        if (nimages <= 0 || nimages > 2000) {
            std::cerr << "Usage: nbodies [nbodies] [nimages] "
                      << "where 0 < nimages <= 2000"
                      << std::endl;
            return 1;
        }
    }

#ifdef SEED1
    // seed random number generator with 1
    rnd.seed(1);
#endif

    Image newimage(MAXW, MAXH);

    // define a force between pixels as if the value of each color (RGB) was a
    // "color-mass" that interacted as a gravitational force that interacts
    // with the "color-mass" of other pixels.

    Body *bodies = new Body[nbodies];

    initialize_bodies(nbodies, bodies, MAXW * SCALE, MAXH * SCALE);
    update_picture(MAXW, MAXH, nbodies, bodies, bkgnd, newimage);
    newimage.write("step0.png");

    cilkview_data_t start, end;
    long long int total_time = 0;
    for (int cnt = 1; cnt< nimages; ++cnt)
    {
      //TODO we want to only profile the time actually calculating
      //not the image writing time, unsure of how to accumulate the resultsi
      //If multiple reports are written with the same tag, Cilk view will plot the smallest one
      //for this it is unclear  if it might be a problem
        __cilkview_query(start);
        for (int i=0; i<NSTEPS; ++i)    {
            calculate_forces(nbodies, bodies);
            update_positions(nbodies, bodies);
#ifdef DEBUG
            dumpbodies(nbodies, bodies);
#endif
        }
        __cilkview_query(end);
        total_time += (end.time - start.time);
        update_picture(MAXW, MAXH, nbodies, bodies, bkgnd, newimage);

        char fname[50];
        std::sprintf(fname, "step%d.png", cnt);
        newimage.write(fname);
    }
    std::cout << "nbodies time: " << total_time << "ms" << std::endl;
    __cilkview_do_report(&start, &end, "nbodies", CV_REPORT_WRITE_TO_LOG | CV_REPORT_WRITE_TO_RESULTS);


    
    
    
    delete[] bodies;

    std::cout << "done." << std::endl;
    return 0;
}
catch (std::exception const& error)
{
    std::cerr << "nbodies: " << error.what() << std::endl;
    return EXIT_FAILURE;
}
