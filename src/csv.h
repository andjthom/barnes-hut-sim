#ifndef _CSV_H
#define _CSV_H

#include <stdio.h>
#include "./constants.h"

/* Read and write properties file in main directory */
int CSV_WritePropFile(const char *dir, int num_particles, int num_steps, float timestep,
                      int steps_per_epoch, int area_size, double init_rot, double mean_rand_vel);
int CSV_ReadPropFile(const char *dir, int *num_particles, int *num_steps, float *timestep,
                     int *steps_per_epoch, int *area_size, double *init_rot, double *mean_rand_vel);
/* Read and write to single files in directory dir */
int CSV_WriteStepFile(const char *dir, const char *columns, int step, int n, Vec3 *data);
int CSV_ReadStepFile(const char *dir, int step, int n, Vec3 *out);
/* Write column information in file filename */
FILE *CSV_InitWrite(const char *filename, const char *columns);
FILE *CSV_InitRead(const char *filename);
void CSV_Close(FILE *file);
/* Read and write information to single file */
int CSV_WriteVec3(FILE *file, int step, Vec3 data);
int CSV_WriteDouble(FILE *file, int step, double data);

#endif
