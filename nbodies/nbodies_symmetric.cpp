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

/* Calculate forces between all of the bodies in the simulation for all pairs */
void calculate_forces(int nbodies, Body *bodies) {
    for (int i = 0; i < nbodies; ++i) {
        for (int j = 0; j < i; ++j) {
            // update the force vector on bodies[i] exerted by bodies[j] and,
            // symmetrically, the force vector on bodies[j] exerted by
            // bodies[i].
            if (i == j) continue;

            double fx, fy;
            calculate_force(&fx, &fy, bodies[i], bodies[j]);
            add_force(&bodies[i], fx, fy);
            add_force(&bodies[j], -fx, -fy);
        }
    }
}

/* Given sums of forces acting on all of the bodies, update their positions */
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
