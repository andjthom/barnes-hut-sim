#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#define G 0.0044995611 /* Big G (in pc^3 M_sol^-1 Myr^-2) */
#define TIMESTEP 0.0001 /* The time that advances in each step of simulation (in Gyr) */
#define THETA 1.0 /* Threshold used in BH algorithm (ratio of distance to center of mass / cell size) */
#define SOFTENING 10.0
#define TOTAL_MASS 1.0e8 /* (in M_sol) */
#define AREA_SIZE 40000 /* (in pc) */
#define STEPS_PER_EPOCH 1000

#define BAR_WIDTH 50

typedef struct {
    double x, y, z;
} Vec3;

#endif
