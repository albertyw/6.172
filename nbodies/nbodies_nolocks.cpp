// nbodies simulation
// Copyright 2009 Cilk Arts
//
// Modified by Cy Chan for 6.172

#include "common.h"

/* traverse the rectangle i0 <= i < i1,  j0 <= j < j1 */
void rect(int i0, int i1, int j0, int j1, Body *bodies)
{
    int di = i1 - i0, dj = j1 - j0;
    const int THRESHOLD = 16;
    if (di > THRESHOLD && dj > THRESHOLD) {
        int im = i0 + di / 2;
        int jm = j0 + dj / 2;
        rect(i0, im, j0, jm, bodies);  // [B]
        rect(im, i1, jm, j1, bodies);  // [B]
        rect(i0, im, jm, j1, bodies);  // [C]
        rect(im, i1, j0, jm, bodies);  // [C]
    }
    else {
        for (int i = i0; i < i1; ++i) {
            for (int j = j0; j < j1; ++j) {
                // update the force vector on bodies[i] exerted by bodies[j]
                // and, symmetrically, the force vector on bodies[j] exerted
                // by bodies[i].
                if (i == j) continue;

                double fx, fy;
                calculate_force(&fx, &fy, bodies[i], bodies[j]);
                add_force(&bodies[i], fx, fy);
                add_force(&bodies[j], -fx, -fy);
            }
        }
    }
}

// traverse the triangle n0 <= i <= j < n1
void triangle(int n0, int n1, Body *bodies)
{
    int dn = n1 - n0;
    if (dn > 1) {
        int nm = n0 + dn / 2;
        triangle(n0, nm, bodies);  // [A]
        triangle(nm, n1, bodies);  // [A]
        rect(n0, nm, nm, n1, bodies);
    }
    else if (dn == 1) {
        // Do nothing.  A single body has no interaction with itself.
    }
}

void calculate_forces(int nbodies, Body *bodies) {
    triangle(0, nbodies, bodies);
}

void update_positions(int nbodies, Body *bodies)
{
    for (int i=0; i<nbodies; ++i) {
        // initial velocity
        double xv0 = bodies[i].xv;
        double yv0 = bodies[i].yv;
        // update velocity based on forces
        bodies[i].xv += TIME_QUANTUM * bodies[i].xf / bodies[i].mass;
        bodies[i].yv += TIME_QUANTUM * bodies[i].yf / bodies[i].mass;
        // clear forces for next iteration
        bodies[i].xf = 0.0;
        bodies[i].yf = 0.0;
        // update position based on average velocity
        bodies[i].x += TIME_QUANTUM * (xv0 + bodies[i].xv)/2.0;
        bodies[i].y += TIME_QUANTUM * (yv0 + bodies[i].yv)/2.0;
    }
}
