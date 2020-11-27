#include <stdlib.h>
#include <string.h>
#include "./csv.h"

int CSV_WritePropFile(const char *prefix, int num_particles, int num_steps, float timestep, int steps_per_epoch,
                      int area_size, double init_rot, double mean_rand_vel)
{
    char filename[256];
    FILE *file;

    sprintf(filename, "%sproperties.csv", prefix);
    file = fopen(filename, "w");

    if (!file)
    {
        printf("Cannot open file: %sproperties.csv\n", prefix);
        return 0;
    }

    fprintf(file, "num_particles,num_steps,timestep,steps_per_epoch,area_size,init_rot,mean_rand_vel\n");
    fprintf(file, "%d,%d,%f,%d,%d,%lf,%lf", num_particles, num_steps, timestep, steps_per_epoch, area_size,
            init_rot, mean_rand_vel);

    fflush(file);
    fclose(file);
    return 1;
}

int CSV_ReadPropFile(const char *prefix, int *num_particles, int *num_steps, float *timestep,
                     int *steps_per_epoch, int *area_size, double *init_rot, double *mean_rand_vel)
{
    char filename[256];
    char line[1024];
    FILE *file;

    sprintf(filename, "%sproperties.csv", prefix);
    file = fopen(filename, "r");

    if (!file)
    {
        printf("Cannot open file: %sproperties.csv\n", prefix);
        return 0;
    }

    fgets(line, 1024, file);
    fscanf(file, "%d,%d,%f,%d,%d,%lf,%lf", num_particles, num_steps, timestep, steps_per_epoch,
           area_size, init_rot, mean_rand_vel);

    fclose(file);
    return 1;
}

int CSV_WriteStepFile(const char *dir, const char *columns, int step, int n, Vec3 *data)
{
    char filename[256];
    FILE *file;
    int i;

    sprintf(filename, "%s%d.csv", dir, step);
    file = fopen(filename, "w");

    if (!file)
    {
        printf("Cannot open file: %s%d.csv\n", dir, step);
        return 0;
    }

    fprintf(file, "%s", columns);

    for (i = 0; i < n; i++)
    {
        fprintf(file, "\n%d,%lf,%lf,%lf", i, data[i].x, data[i].y, data[i].z);
    }

    fflush(file);
    fclose(file);
    return 1;
}

int CSV_ReadStepFile(const char *dir, int step, int n, Vec3 *out)
{
    char filename[256];
    char line[1024];
    FILE *file;
    Vec3 vec;
    int i, p;

    sprintf(filename, "%s%d.csv", dir, step);
    file = fopen(filename, "r");

    if (!file)
    {
        printf("Cannot open file: %s%d.csv\n", dir, step);
        return 0;
    }

    fgets(line, 1024, file);
    fscanf(file, "%d,%lf,%lf,%lf", &p, &vec.x, &vec.y, &vec.z);
    out[0] = vec;

    for (i = 1; i < n; i++)
    {
        fscanf(file, "\n%d,%lf,%lf,%lf", &p, &vec.x, &vec.y, &vec.z);
        out[p] = vec;
    }

    fclose(file);
    return 1;
}

FILE *CSV_InitWrite(const char *filename, const char *columns)
{
    FILE *file;

    file = fopen(filename, "w");
    if (!file)
    {
        printf("Cannot open: %s\n", filename);
        return NULL;
    }

    fprintf(file, "%s", columns);
    return file;
}

FILE *CSV_InitRead(const char *filename)
{
    FILE *file;
    char line[1024];

    file = fopen(filename, "r");
    if (!file)
    {
        printf("Cannot open: %s\n", filename);
        return NULL;
    }

    fgets(line, 1024, file);
    return file;
}

void CSV_Close(FILE *file)
{
    fflush(file);
    fclose(file);
}

int CSV_WriteVec3(FILE *file, int step, Vec3 data)
{
    return fprintf(file, "\n%d,%lf,%lf,%lf", step, data.x, data.y, data.z);
}

int CSV_WriteDouble(FILE *file, int step, double data)
{
    return fprintf(file, "\n%d,%lf", step, data);
}
